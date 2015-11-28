var wshell = new ActiveXObject("WScript.Shell");
var xmlhttp = new ActiveXObject("MSXML2.ServerXMLHTTP");
var adodb = new ActiveXObject("ADODB.Stream");
var FSO = new ActiveXObject("Scripting.FileSystemObject");

function http_get(url, is_binary)
{
	xmlhttp.open("GET", url);
	xmlhttp.setRequestHeader("User-Agent", "curl/7.21.2 (i386-pc-win32) libcurl/7.21.2 OpenSSL/0.9.8o zlib/1.2.5");
	xmlhttp.send();
	WScript.echo("retrieving " + url);
	while (xmlhttp.readyState != 4);
	WScript.Sleep(10);
	if (xmlhttp.status != 200)
	{
		WScript.Echo("http get failed: " + xmlhttp.status);
		WScript.Quit(2)
	};
	return is_binary ? xmlhttp.responseBody : xmlhttp.responseText;
};
function save_binary(path, data)
{
	adodb.type = 1;
	adodb.open();
	adodb.write(data);
	adodb.saveToFile(path, 2);
};

var base_url = WScript.Arguments(0);
var filename = WScript.Arguments(1);
var bin_data = http_get(base_url, true);
save_binary(filename, bin_data);

	