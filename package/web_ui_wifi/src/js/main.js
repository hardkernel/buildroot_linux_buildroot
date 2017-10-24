var obj_Data;


function HTMLEnCode(str)
{
     var    s    =    "";
     if (str.length    ==    0)    return    "";
     s = str.replace(/&/g, "&gt;");
     s = s.replace(/</g, "&lt;");
     s = s.replace(/>/g, "&gt;");
     s = s.replace(/ /g, "&nbsp;");
     s = s.replace(/\'/g, "'");
     s = s.replace(/\"/g, "&quot;");
     s = s.replace(/\n/g, "<br>");
     return    s;
};


function handleCommand(command_xmlhttp, callback)
{
  if(command_xmlhttp.readyState==4)
    {
      if(command_xmlhttp.status == 200){
    	if(callback == null)
    		return;
	callback(command_xmlhttp);
     }
  }
};

 function send_commond(command, callback)
 {
    command_xmlhttp = GetXmlHttpObject();               //create req obj
    command_xmlhttp.onreadystatechange = function () {        //get data from web action
        handleCommand(command_xmlhttp, callback);
    }

    var url="cgi-bin/wifi/wifi.cgi?cmd=" + encodeURIComponent(command);
    command_xmlhttp.open("GET", url ,true);
    command_xmlhttp.setRequestHeader("If-Modified-Since","0");
    command_xmlhttp.setRequestHeader("Cache-Control","no-cache");
    command_xmlhttp.setRequestHeader("CONTENT-TYPE","text/plain");
    command_xmlhttp.send(null);
    //command_xmlhttp.send("cmd="+command);
 };

function get_wifi_list()
 {
 	var wifi_list = null;
 	var wifi_json = send_commond("get_wifi_list", set_wifi_list_to_select);
 };

function set_wifi(ssid,pwd)
{
	var wifi_ssid = ssid;
	var wifi_pwd = pwd;
	var wifi_json = send_commond("set_wifi&ssid="+wifi_ssid+"&"+"pwd="+wifi_pwd);
};

function set_wifi_list_to_select(command_xmlhttp)
{
	var response_date = command_xmlhttp.responseText;

	obj_Data = eval("("+response_date+")");
	if(obj_Data.length == 0){
		get_wifi_list();
	}
	else{
        var htmlNodes = '';

        for(var i = 0; i < obj_Data.length; i++){
            htmlNodes += '<a class="list-group-item" id="wifi_' + i + '"' + ' role="button" data-toggle="modal" data-target="#myModal">' + obj_Data[i].ssid + '</a>';
        }
        htmlNodes += '</ul>';

        $('#testtext').html(htmlNodes);

        var j;
        var index = 1;
        var wifi_select = document.getElementById("ssid");
        for(j = 0; j < obj_Data.length; j++){
            var wifi = new Object;
            wifi.ssid = obj_Data.aplist[j].ssid;
            wifi_select.Option.add(wifi.id);
        }
	}
};

function set_wifi_by_input()
{
    var input_ssid = document.getElementById("selfModal_ssid").value;
    var input_pwd = document.getElementById("selfModal_pwd").value;
    set_wifi(input_ssid,input_pwd);
}


$('ul').on('click','a',function(){
	var inner_ssid = this.innerText;
	var my_ssid = document.getElementById("modal_ssid");
	var my_pwd = document.getElementById("modal_pwd");
	my_pwd.value = "";
	my_ssid.value = inner_ssid;
});


function set_wifi_by_modal_input()
{
    setTimeout(function(){$("#mymodal").modal("hide")},2000);
    var input_ssid = document.getElementById("modal_ssid").value;
    var input_pwd = document.getElementById("modal_pwd").value;
    // if (check_input_is_ok(input_ssid))
    //     window.setTimeout(function(){
    // $('#alertTest').modal({
    //     backdrop:true,
    //     keyboard:true,
    //     show:true
    //     });
    // return false;
    // },2000);
    // else
    set_wifi(input_ssid,input_pwd);
};

$('#refresh_img').click(function(){
	get_wifi_list();
});

function check_input_is_ok(input_text)
{
    if (input_text=="")
        return false;
    else
        return true;
};

function set_spotify(){
	var spotify_username = document.getElementById("modal_spotify_username").value;
	var spotify_pwd = document.getElementById("modal_spotify_pwd").value;
	var spotify_dname = document.getElementById("modal_spotify_dname").value;

	send_commond("set_spotify&username="+spotify_username+"&"+"spotify_pwd="+spotify_pwd+"&"+"spotify_dname="+spotify_dname);
	var showUserId = '<h3 style="color:green" align="center">Logging...</h3>';
	$('#userinfo').html(showUserId);
	setTimeout("check_spotify()",3000);
};

function TurnOffSpotify(){
	send_commond("kill_spotify");
	var showStopping = '<h3 style="color:red" align="center">stopping...</h3>';
	$('#userinfo').html(showStopping);
	setTimeout("check_spotify()",3000);
};

function check_spotify(){
	send_commond("check_spotify",handle_check);
};

function handle_check(command_xmlhttp){

	var response_check_date = command_xmlhttp.responseText;

	if(response_check_date == "")
		return;

	if(response_check_date.length > 7){
		send_commond("get_spo_info",handle_get_info);
	}
	else{
		var htmlNodes_spo_info = '<h3 style="color:red" align="center">Stopped</h3>';
		$('#userinfo').html(htmlNodes_spo_info);
	}
};

function handle_get_info(){
	var response_spo_info = command_xmlhttp.responseText;
	if(response_spo_info == "")
		return;
	var spo_info = eval("("+response_spo_info+")");
	console.log(spo_info);
	var old_uname = spo_info[0].infos;
	var old_dname = spo_info[2].infos;

	var htmlNodes_spo_info = '<h3 style="color:green" align="center">user: '+old_uname+' device: '+old_dname+'</h3>';
	$('#userinfo').html(htmlNodes_spo_info);
};

function system(){

	var swupdate_href = '<li><a href="http://'+window.location.host+':8080" class="list-group-item" role="button">Swupdate</a></li>';
	$('#swupdate').html(swupdate_href);

	send_commond("get_deviceinfo",show_device_info);
};
function show_device_info(){

	send_commond("runswupdate");
	var response_dev_info = command_xmlhttp.responseText;
	if(response_dev_info == "")
		return;
	var dev_info = eval("("+response_dev_info+")");
	var htmlNodes_dev_info = '';

	htmlNodes_dev_info += '<li class="list-group-item" align="center" >KERNEL : ' + dev_info[0].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >ARCH : ' + dev_info[8].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >SSID : ' + dev_info[1].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >MODE : ' + dev_info[2].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >CIPHER : ' + dev_info[3].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >KEYMGMT : ' + dev_info[4].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >STATE : ' + dev_info[5].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >WIREIP : ' + dev_info[6].info+'</a></li>'
	htmlNodes_dev_info += '<li class="list-group-item" align="center" >MAC; : ' + dev_info[7].info+'</a></li>'

	$('#dev_info_show').append(htmlNodes_dev_info);
};

function stop_swupdate(){
	send_commond("endswupdate");
}
