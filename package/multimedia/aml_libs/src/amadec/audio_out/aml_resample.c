

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <linux/fb.h>
#include <log-print.h>
#include "aml_resample.h"


af_resampe_ctl_t af_resampler_ctx={0};

static int get_sysfs_int(const char *path)
{
    int fd;
    int val = 0;
    char  bcmd[8]={0};;
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, bcmd, 16);
        if (bcmd[0]=='O' && bcmd[1]=='N')    val=1;
            
        else if(bcmd[0]=='O'&& bcmd[1]=='F') val=0;

        else if(bcmd[0]=='N'&& bcmd[1]=='O')  val=0;

        else if(bcmd[0]=='D'&& bcmd[1]=='W')  val=1;
            
        else if(bcmd[0]=='U'&& bcmd[1]=='P')  val=2;

        //adec_print("value=%s coresponding val=%d\n",bcmd,val);
        
        close(fd);
    }else{
        //adec_print("open %s failed\n",path);
    }
    return val;
}

static int  get_sysfs_str(const char *path, char *valstr, int size)
{
    int fd;
    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, valstr, size - 1);
        valstr[strlen(valstr)] = '\0';
        close(fd);
    } else {
        sprintf(valstr, "%s", "fail");
        return -1;
    };
    log_print("get_sysfs_str=%s\n", valstr);
    return 0;
}


int af_get_reample_enable_flag()
{
    return get_sysfs_int("sys/class/amaudio/enable_resample");
}

int af_get_resample_type()
{
    return get_sysfs_int("sys/class/amaudio/resample_type");
}



af_resampe_ctl_t* af_resampler_ctx_get()
{  
    return &af_resampler_ctx;
}

static void af_resample_linear_coef_get(af_resampe_ctl_t *paf_resampe_ctl)
{
    int SampNumIn=paf_resampe_ctl->SampNumIn;
	int SampNumOut=paf_resampe_ctl->SampNumOut;
	int *pCoefArray=paf_resampe_ctl->InterpolateCoefArray;
	short *pindex=paf_resampe_ctl->InterpolateIndexArray;
    int i;
    int step_i= (((SampNumIn-1))<<14)/(SampNumOut-1);    
    int step_forward=0;
	int insert_pos_int=0;
    
    if(SampNumIn==SampNumOut){
       for (i=0;i<SampNumOut;i++){
           pindex[i]=i;
           pCoefArray[i]=0;
        }
        return;
    } 
    
    step_forward=0;
	for (i=1;i<=SampNumOut-2;i++){
        step_forward+=step_i;
        insert_pos_int +=Q14_INT_GET(step_forward);
        pindex[i]=insert_pos_int;
        pCoefArray[i]=Q14_FRA_GET(step_forward);
        step_forward = Q14_FRA_GET(step_forward);
	}
	pindex[0]=0;
    pCoefArray[0]=0;
	pindex[SampNumOut-1]=SampNumOut-1;
	pCoefArray[SampNumOut-1]=0;
}


void af_resample_set_SampsNumRatio(af_resampe_ctl_t *paf_resampe_ctl)
{  
    int resample_type=af_get_resample_type();
	paf_resampe_ctl->LastResamType=resample_type;
	adec_print("ReSample Coef Init: type/%d",resample_type);
    //memset(paf_resampe_ctl,0,sizeof(af_resampe_ctl_t));
    if(resample_type==RESAMPLE_TYPE_NONE){
         paf_resampe_ctl->SampNumIn=DEFALT_NUMSAMPS_PERCH;
         paf_resampe_ctl->SampNumOut=DEFALT_NUMSAMPS_PERCH;
    }
    else if(resample_type==RESAMPLE_TYPE_DOWN)
    {
         paf_resampe_ctl->SampNumIn=DEFALT_NUMSAMPS_PERCH + RESAMPLE_DELTA_NUMSAMPS;
         paf_resampe_ctl->SampNumOut=DEFALT_NUMSAMPS_PERCH ;
    }
    else if(resample_type==RESAMPLE_TYPE_UP)
    {
         paf_resampe_ctl->SampNumIn=DEFALT_NUMSAMPS_PERCH - RESAMPLE_DELTA_NUMSAMPS;
         paf_resampe_ctl->SampNumOut=DEFALT_NUMSAMPS_PERCH;
    }
    af_resample_linear_coef_get(paf_resampe_ctl);
    paf_resampe_ctl->ResevedSampsValid=0;
    paf_resampe_ctl->OutSampReserveLen=0;
    paf_resampe_ctl->InitFlag=1;

}

void af_resample_linear_init()
{
    af_resampe_ctl_t *paf_resampe_ctl;
	paf_resampe_ctl=&af_resampler_ctx;
	memset(paf_resampe_ctl,0,sizeof(af_resampe_ctl_t));
	adec_print("af_resample_linear_init");
}


int af_get_delta_inputsampnum(af_resampe_ctl_t *paf_resampe_ctl,int Nch)
{
   return  paf_resampe_ctl->SampNumIn*Nch - paf_resampe_ctl->ResevedSampsValid;

}

void  af_get_unpro_inputsampnum(af_resampe_ctl_t *paf_resampe_ctl,short *buf, int *num)
{
     if(*num >= paf_resampe_ctl->ResevedSampsValid )
     {
        memcpy(buf,paf_resampe_ctl->ResevedBuf,paf_resampe_ctl->ResevedSampsValid*sizeof(short));
        *num = paf_resampe_ctl->ResevedSampsValid;
        paf_resampe_ctl->ResevedSampsValid=0;
        
     }else{//*num < paf_resampe_ctl->ResevedSampsValid
        memcpy(buf,paf_resampe_ctl->ResevedBuf,(*num)*sizeof(short));
        memmove(paf_resampe_ctl->ResevedBuf,
                paf_resampe_ctl->ResevedBuf+(*num),
                (paf_resampe_ctl->ResevedSampsValid-(*num))*sizeof(short)
               );
        paf_resampe_ctl->ResevedSampsValid -= (*num);
     }

}



void af_get_pcm_in_resampler(af_resampe_ctl_t *paf_resampe_ctl,short*buf,int *len)
{
   int NumSamp_out =*len;
   int rest_pcm_nums=0;
   if((NumSamp_out>=0) && (NumSamp_out < paf_resampe_ctl->OutSampReserveLen))
   {    
        rest_pcm_nums = paf_resampe_ctl->OutSampReserveLen - NumSamp_out;
	    memcpy(buf,paf_resampe_ctl->OutSampReserveBuf,NumSamp_out*sizeof(short));
        memmove(paf_resampe_ctl->OutSampReserveBuf,
				paf_resampe_ctl->OutSampReserveBuf+NumSamp_out,
				rest_pcm_nums*sizeof(short));
        
   }else if(NumSamp_out>=paf_resampe_ctl->OutSampReserveLen){
		memcpy(buf,paf_resampe_ctl->OutSampReserveBuf,
				   paf_resampe_ctl->OutSampReserveLen*sizeof(short));
		NumSamp_out   = paf_resampe_ctl->OutSampReserveLen;
        rest_pcm_nums = 0;
   }
   *len=NumSamp_out;
   paf_resampe_ctl->OutSampReserveLen=rest_pcm_nums;
}

void  af_resample_process_linear_inner(af_resampe_ctl_t *paf_resampe_ctl,short *data_in, int *NumSamp_in,short* data_out,int* NumSamp_out,int NumCh)
{    
    
	int index,ChId;
	int NumSampsPerCh_in=(*NumSamp_in)/NumCh;
    int NumSampsPerCh_Pre=paf_resampe_ctl->ResevedSampsValid/NumCh;
    
    int   NumSampsPerCh_out;
	short buf16_in[MAX_NUMSAMPS_PERCH];
    short buf16_in_valid;
    short *pPreSamps=paf_resampe_ctl->ResevedBuf;
	short *pindex=paf_resampe_ctl->InterpolateIndexArray;
	int   *pcoef=paf_resampe_ctl->InterpolateCoefArray;
    int   input_offset=0,output_offset=0;
    int   cur_out_samp_reserve_num=0;
     
    if (!paf_resampe_ctl->InitFlag)
    {
       af_resample_set_SampsNumRatio(paf_resampe_ctl);
    }
    //indecate at the end of pcm stream
    if( NumSampsPerCh_Pre + NumSampsPerCh_in < paf_resampe_ctl->SampNumIn ){
        memcpy(pPreSamps+paf_resampe_ctl->ResevedSampsValid,data_in,(*NumSamp_in)*sizeof(short));
        paf_resampe_ctl->ResevedSampsValid += (*NumSamp_in);
        //memcpy(data_out,paf_resampe_ctl->ResevedBuf,paf_resampe_ctl->ResevedSampsValid);
        //*NumSamp_out=paf_resampe_ctl->ResevedSampsValid;
        *NumSamp_out=0;
    }else{
         int NumSampsPerCh_Rest=NumSampsPerCh_Pre + NumSampsPerCh_in -paf_resampe_ctl->SampNumIn;
         input_offset +=(paf_resampe_ctl->SampNumIn-NumSampsPerCh_Pre)*NumCh;
         output_offset+= paf_resampe_ctl->OutSampReserveLen;
         memcpy(pPreSamps+paf_resampe_ctl->ResevedSampsValid,data_in,sizeof(short)*input_offset);
         memcpy(data_out,paf_resampe_ctl->OutSampReserveBuf,sizeof(short)*paf_resampe_ctl->OutSampReserveLen);
         buf16_in_valid=paf_resampe_ctl->SampNumIn;
         for(ChId=0;ChId<NumCh;ChId++)
	     {  
		      for(index=0;index<buf16_in_valid;index++)
                   buf16_in[index]= pPreSamps[NumCh*index+ChId];
		      for(index=0;index<buf16_in_valid;index++)
		      {   int pos=pindex[index];
		          short t16;
	              t16 =buf16_in[pos]+Q14_INT_GET((pcoef[index]*(buf16_in[pos+1]-buf16_in[pos])));
                  data_out[output_offset+NumCh*index+ChId]=t16;
		      }			
		      data_out[output_offset+NumCh*(paf_resampe_ctl->SampNumOut-1)+ChId]=buf16_in[buf16_in_valid-1];
         }
         output_offset +=paf_resampe_ctl->SampNumOut*NumCh;
         paf_resampe_ctl->ResevedSampsValid = 0;
         
         while(NumSampsPerCh_Rest > paf_resampe_ctl->SampNumIn)
         {
              buf16_in_valid=paf_resampe_ctl->SampNumIn;
              for(ChId=0;ChId<NumCh;ChId++)
	          {   
		           for(index=0;index<buf16_in_valid;index++)
                         buf16_in[index]= data_in[input_offset+NumCh*index+ChId];
		           for(index=0;index<buf16_in_valid;index++)
		           {   int pos=pindex[index];
		               short t16;
	                   t16 =buf16_in[pos]+Q14_INT_GET(((int64_t)pcoef[index]*(buf16_in[pos+1]-buf16_in[pos])));
                       data_out[output_offset+NumCh*index+ChId]=t16;
		           }			
		           data_out[output_offset+NumCh*(paf_resampe_ctl->SampNumOut-1)+ChId]=buf16_in[buf16_in_valid-1];
              }
              NumSampsPerCh_Rest -= paf_resampe_ctl->SampNumIn;
              input_offset +=paf_resampe_ctl->SampNumIn*NumCh;
              output_offset +=paf_resampe_ctl->SampNumOut*NumCh;
         }
         cur_out_samp_reserve_num = output_offset%DEFALT_NUMSAMPS_PERCH ;
         paf_resampe_ctl->OutSampReserveLen = cur_out_samp_reserve_num;
        
         memcpy(paf_resampe_ctl->OutSampReserveBuf,
                data_out+output_offset-cur_out_samp_reserve_num,
                sizeof(short)*cur_out_samp_reserve_num);
        *NumSamp_out=output_offset-cur_out_samp_reserve_num;
  
         if(input_offset< (*NumSamp_in)){
            memcpy(pPreSamps,data_in+input_offset,sizeof(short)*NumSampsPerCh_Rest*NumCh);
            paf_resampe_ctl->ResevedSampsValid=NumSampsPerCh_Rest*NumCh;
         }else{
            paf_resampe_ctl->ResevedSampsValid=0;
         }
    }
}

void  af_resample_stop_process(af_resampe_ctl_t *paf_resampe_ctl)
{
    //memcpy(pPreservedSamps,paf_resampe_ctl->OutSampReserveBuf,sizeof(short)*paf_resampe_ctl->OutSampReserveLen);
    //memcpy(pPreservedSamps+paf_resampe_ctl->OutSampReserveLen,paf_resampe_ctl->ResevedBuf,sizeof(short)*paf_resampe_ctl->ResevedSampsValid);
    //*SampNum=paf_resampe_ctl->OutSampReserveLen + paf_resampe_ctl->ResevedSampsValid;
    // paf_resampe_ctl->ResevedSampsValid=0;
     //paf_resampe_ctl->OutSampReserveLen=0;
     paf_resampe_ctl->InitFlag=0;
	 paf_resampe_ctl->LastResamType=0;
    // adec_print("resample stop INIT_FLAG=%d\n",paf_resampe_ctl->InitFlag);
}

#define MAXCH_NUMBER 8
#define MAXFRAMESIZE 8192
short  date_temp[MAXCH_NUMBER*MAXFRAMESIZE];
extern int android_reset_track(struct aml_audio_dec* audec);
/**
 *  try to read as much data as len from dsp buffer
 */
static int dsp_pcm_read(aml_audio_dec_t*audec,char *data_in,int len)
{
   int pcm_ret=0,pcm_cnt_bytes=0;
   int wait_times=0;
   while(pcm_cnt_bytes<len){
      pcm_ret=audec->adsp_ops.dsp_read(&audec->adsp_ops, data_in+pcm_cnt_bytes, len-pcm_cnt_bytes);
	  if(pcm_ret==0){//indicate there is no data in dsp_buf,still try to read more data:
          wait_times++;
		  usleep(20);
		  if(wait_times>50){
		    adec_print("NOTE:dsp no enough data!");
		    break;
		  }
	  }
	  pcm_cnt_bytes+=pcm_ret;
   }
   return pcm_cnt_bytes/sizeof(short);
}

static void dump_pcm_bin(char *path,char *buf,int size)
{
	FILE *fp=fopen(path,"ab+");
	 if(fp!= NULL){
		   fwrite(buf,1,size,fp);
		   fclose(fp);
	}
}


void af_resample_api(char *buffer, unsigned int *size,int Chnum, aml_audio_dec_t *audec)
{
	int len;
	int resample_enable;
	af_resampe_ctl_t *paf_resampe_ctl;
	short data_in[MAX_NUMSAMPS_PERCH*DEFALT_NUMCH], *data_out;
	short outbuftmp16[MAX_NUMSAMPS_PERCH*DEFALT_NUMCH];
    int NumSamp_in,NumSamp_out,NumCh,NumSampRequir=0;
	static int print_flag=0;
	int outbuf_offset=0;
	int dsp_format_changed_flag=0;
	static int pcm_left_len=-1;
       //------------------------------------------
        NumCh=Chnum;//buffer->channelCount;		
        resample_enable= af_get_reample_enable_flag();
		paf_resampe_ctl=af_resampler_ctx_get();
        data_out=date_temp;//buffer->i16
		NumSamp_out = *size/sizeof(short);//buffer->size/sizeof(short);
		if(NumSamp_out>MAXCH_NUMBER*MAXFRAMESIZE)
			NumSamp_out=MAXCH_NUMBER*MAXFRAMESIZE;
		NumSampRequir=NumSamp_out;
		int resample_type=af_get_resample_type();
		
		//adec_print("REQURE_NUM-----------------------------%d\n",NumSamp_out);
		
		//-------------------------------
		//add this part for support rapid Resample_Type covet:
		if (resample_enable)
		 {   
		 	if(paf_resampe_ctl->LastResamType!=resample_type)
		 	{ 
				adec_print("ReSample Type Changed: FromTYpe/%d ToType/%d \n",
					         paf_resampe_ctl->LastResamType,resample_type);
			    if((paf_resampe_ctl->OutSampReserveLen==0) // to ensure phase continue:
		   	      && (paf_resampe_ctl->ResevedSampsValid==0))
		        {   
					adec_print("ReSample Type Changed: ENABLE");
                    af_resample_stop_process(paf_resampe_ctl);
		        }else{
		            adec_print("ReSample Type Changed DISABLE:");
					adec_print("  OutSampSaved/%d InSampSaved/%d in Resampler!",
						        paf_resampe_ctl->OutSampReserveLen,paf_resampe_ctl->ResevedSampsValid);
		   	        resample_enable=0;
		        }

		 	 }
		 }
        //-------------------------------
        if(resample_enable){    
			   int pcm_cnt=0;
			   if (!paf_resampe_ctl->InitFlag)
                   af_resample_set_SampsNumRatio(paf_resampe_ctl);
			   af_get_pcm_in_resampler(paf_resampe_ctl,data_out+outbuf_offset,&NumSampRequir);
			   
			   //adec_print("RETURN_SIZE_1:%d    OutSampReserve=%d \n",NumSampRequir,paf_resampe_ctl->OutSampReserveLen);

			   outbuf_offset += NumSampRequir;
			   NumSamp_out   -= NumSampRequir;
			   while(NumSamp_out >= DEFALT_NUMSAMPS_PERCH*NumCh)
			   {   
			       int delta_input_sampsnum=af_get_delta_inputsampnum(paf_resampe_ctl,NumCh);
			       NumSamp_in =dsp_pcm_read(audec,(char*)data_in,delta_input_sampsnum*sizeof(short));	
				   af_resample_process_linear_inner(paf_resampe_ctl,data_in, &NumSamp_in,data_out+outbuf_offset,&pcm_cnt,NumCh);

				   //adec_print("RETURN_SIZE_2:%d    OutSampReserve=%d \n",pcm_cnt,paf_resampe_ctl->OutSampReserveLen);

				   if(pcm_cnt==0)
				   	  goto resample_out;
				   outbuf_offset += pcm_cnt;
				   NumSamp_out   -= pcm_cnt;
			   }
			   
               if(NumSamp_out>0)
			   {
			       int delta_input_sampsnum=af_get_delta_inputsampnum(paf_resampe_ctl,NumCh);
			       NumSamp_in =dsp_pcm_read(audec,(char*)data_in,delta_input_sampsnum*sizeof(short));				   
            	   af_resample_process_linear_inner(paf_resampe_ctl,data_in, &NumSamp_in,outbuftmp16,&pcm_cnt,NumCh);
                   if(pcm_cnt==0)
				   	   goto resample_out;

				   //adec_print("RETURN_SIZE_3:%d    OutSampReserve=%d \n",NumSamp_out,pcm_cnt-NumSamp_out);
   
				   memcpy(data_out+outbuf_offset,outbuftmp16,NumSamp_out*sizeof(short));
				   outbuf_offset +=NumSamp_out;
                   memcpy(paf_resampe_ctl->OutSampReserveBuf,outbuftmp16+NumSamp_out,(pcm_cnt-NumSamp_out)*sizeof(short));
                   paf_resampe_ctl->OutSampReserveLen = (pcm_cnt-NumSamp_out);
			   }
			   
		}else{
              if(paf_resampe_ctl->OutSampReserveLen > 0){
			      af_get_pcm_in_resampler(paf_resampe_ctl,data_out+outbuf_offset,&NumSampRequir);
				  //adec_print("RETURN_SIZE_4:%d    OutSampReserve=%d \n",NumSampRequir,paf_resampe_ctl->OutSampReserveLen);
			      outbuf_offset += NumSampRequir;
			      NumSamp_out   -= NumSampRequir;
				  NumSampRequir  =  NumSamp_out;
              } 

			  if(paf_resampe_ctl->ResevedSampsValid > 0){
                  af_get_unpro_inputsampnum(paf_resampe_ctl,data_out+outbuf_offset,&NumSampRequir);
				  //adec_print("RETURN_SIZE_5:%d    OutSampReserve=%d \n",NumSampRequir,paf_resampe_ctl->ResevedSampsValid);
				  outbuf_offset += NumSampRequir;
			      NumSamp_out   -= NumSampRequir;
			  }
			  
              if((paf_resampe_ctl->OutSampReserveLen==0) && (paf_resampe_ctl->ResevedSampsValid==0))
                  af_resample_stop_process(paf_resampe_ctl);

			  if(NumSamp_out > 0){
			  	  len=audec->adsp_ops.dsp_read(&audec->adsp_ops, (char*)(data_out+outbuf_offset),NumSamp_out*sizeof(short));
                  outbuf_offset += len/sizeof(short);
			  }
        }
	resample_out:
		
	    *size=outbuf_offset*sizeof(short);
		memcpy(buffer,data_out,*size);
		//------------------------------------
        dsp_format_changed_flag=audiodsp_format_update(audec);
	    if(dsp_format_changed_flag>0){
			pcm_left_len=audiodsp_get_pcm_left_len();
	    }
		if(pcm_left_len>=0){
			if(pcm_left_len>(*size)){
				pcm_left_len-=(*size);
				memset((char*)(data_out),0,*size);
			}else if(pcm_left_len<=(*size)){
			    memset((char*)(data_out),0,pcm_left_len);
				pcm_left_len=-1;	
			
			}
		}
		//dump_pcm_bin("/data/post1.pcm",(char*)(buffer->i16),buffer->size);
		//---------------------------------------------

}

