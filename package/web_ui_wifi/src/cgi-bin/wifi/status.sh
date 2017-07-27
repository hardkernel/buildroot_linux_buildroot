#!/bin/sh
PATH=/bin:/sbin:/usr/bin:/usr/sbin
echo "Content-type: text/html"
echo ""
echo "<html>"
echo "<body>"
echo "<h3>Connected Status</h3>"
echo '<button onclick="location.reload();">Flash</button></p>'
echo '<h1 id = "status" hidden>'
rd=$(wpa_cli status)
echo $rd
echo "</h1>"
echo "<script>"
echo 'var lab = document.getElementById("status");'
echo "var str = lab.innerHTML;"
echo 'var strs = str.split(" ");'
echo "for(var i in strs){"
echo 'document.write(strs[i]+"<br>");}'
echo "</script>"
echo "</body>"
echo "</html>"
