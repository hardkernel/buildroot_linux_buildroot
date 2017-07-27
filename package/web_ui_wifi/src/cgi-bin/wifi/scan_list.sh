#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
echo "Content-type: text/html"
echo ""
echo "<html>"
echo "<body>"
echo '<h3 align="center">WIFI LIST</h3>'
echo '<p align="center"><button onclick="location.reload();">Flash</button></p>'
echo '<h1 id = "h1" hidden>'
wpa_cli  scan > /dev/null
rd=$(wpa_cli  scan_result | awk '{print $5}')
echo $rd
echo "</h1>"
echo "<script>"
echo 'var lab = document.getElementById("h1");'
echo "var str = lab.innerHTML;"
echo 'var strs = str.split(" ");'
echo "for(var i in strs){"
echo 'document.write(strs[i]+"<br>");}'
echo "</script>"
echo "</body>"
echo "</html>"
