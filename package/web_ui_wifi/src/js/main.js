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
	if(response_date == "")
		return;
	obj_Data = eval("("+response_date+")");
	console.log(obj_Data);
	var htmlNodes = '';

	for(var i = 0; i < obj_Data.length; i++){
		htmlNodes += '<a class="list-group-item" id="wifi_' + i + '"' + ' role="button" data-toggle="modal" data-target="#myModal">' + obj_Data[i].ssid + '</a>';
	}
	htmlNodes += '</ul>';
	console.log(htmlNodes);
	$('#testtext').append(htmlNodes);

	var j;
	var index = 1;
	var wifi_select = document.getElementById("ssid");
	for(j = 0; j < obj_Data.length; j++){
		var wifi = new Object;
		wifi.ssid = obj_Data.aplist[j].ssid;
		wifi_select.Option.add(wifi.id);
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

window.onload = function(){
	get_wifi_list();
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

