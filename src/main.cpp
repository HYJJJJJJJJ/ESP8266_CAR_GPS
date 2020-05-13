#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Ticker.h> //Ticker Library.

TinyGPSPlus gps;
static const int RXPin = 12, TXPin = 14;        //d6,d5
SoftwareSerial swSer(RXPin, TXPin, false, 256); //rx,tx,
static const uint32_t GPSBaud = 9600;

const char *ssid = "HYJ";
const char *password = "h103103103";
// Set web server port number to 80
WiFiServer server(80);
String header; // Variable to store the HTTP request
double LAT = 0.000000;
double LNG = 0.000000;
const int output4 = D4;
char out_str[100] = {0};

String readString = ""; //建立一个字符串对象用来接收存放来自客户的数据

//响应头
String responseHeaders =
    String("") +
    "HTTP/1.1 200 OK\r\n" +
    "Content-Type: text/html\r\n" +
    "Connection: close\r\n" +
    "\r\n";

//网页
String myhtmlPage =
    String("") +
    "<!DOCTYPE html>\r\n" +
    "<html>\r\n" +
    "<head>\r\n" +
    "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />\r\n" +
    "    <meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" />\r\n" +
    "    <style type=\"text/css\">\r\n" +
    "        body,\r\n" +
    "        html {\r\n" +
    "            width: 100%;\r\n" +
    "            height: 100%;\r\n" +
    "            margin: 0;\r\n" +
    "            font-family: \"微软雅黑\";\r\n" +
    "        }\r\n" +
    "        #allmap {\r\n" +
    "            height: 500px;\r\n" +
    "            width: 100%;\r\n" +
    "        }\r\n" +
    "        #r-result {\r\n" +
    "            width: 100%;\r\n" +
    "        }\r\n" +
    "    </style>\r\n" +
    "    <script type=\"text/javascript\" src=\"//api.map.baidu.com/api?v=2.0&ak=0NALIoVw7xmacSguGmUSxG9hVMRTYbSE\"></script>\r\n" +
    "    <title>车载定位系统</title>\r\n" +
    "</head>\r\n" +
    "<body>\r\n" +
    "    <div id=\"allmap\" style=\"height:900px\"></div>\r\n" +
    "    <div id=\"r-result\">\r\n" +
    "        <b id=\"p1\">\r\n" +
    "            <pre>    </pre>纬度：</b>\r\n" +
    "        <b id=\"p2\">\r\n" +
    "            <pre>    </pre>经度：</b>\r\n" +
    "    </div>\r\n" +
    "    <script>\r\n" +
    "        var str = new Array();\r\n" +
    "        var xmlhttp;\r\n" +
    "        xmlhttp = new XMLHttpRequest();\r\n" +
    "        var firsttime = 0;\r\n" +
    "        var getposition = window.setInterval(\"get_data()\", 1000);\r\n" +
    "        //var updateposition = window.setInterval(\"update_marker()\", 1000);\r\n" +
    "        function callback() {\r\n" +
    "            console.log(\"CallBack:%d\", xmlhttp.readyState);\r\n" +
    "            if (xmlhttp.readyState == 4) {\r\n" +
    "                if (xmlhttp.status == 200) {\r\n" +
    "                    var responseTex = xmlhttp.responseText;\r\n" +
    "                    console.log(\"Response is %s\", responseTex);\r\n" +
    "                    str = responseTex.split(\",\");\r\n" +
    "                    update_marker();\r\n" +
    "                }\r\n" +
    "            }\r\n" +
    "        }\r\n" +
    "        function  get_data() {\r\n" +
    "            if (window.XMLHttpRequest) {\r\n" +
    "                xmlhttp = new XMLHttpRequest();\r\n" +
    "                if (xmlhttp.overrideMimeType) {\r\n" +
    "                    xmlhttp.overrideMimeType(\"text/xml\")\r\n" +
    "                }\r\n" +
    "            } else if (window.ActiveXObject) {\r\n" +
    "                var activexName = [\"MSXML2.XMLHTTP\", \"Microsoft.XMLHTTP\"];\r\n" +
    "                for (var i = 0; i < activexName.length; i++) {\r\n" +
    "                    try {\r\n" +
    "                        xmlhttp = new ActiveXObject(activexName[i]);\r\n" +
    "                        break;\r\n" +
    "                    } catch (e) {\r\n" +
    "                    }\r\n" +
    "                }\r\n" +
    "            }\r\n" +
    "            xmlhttp.onreadystatechange = callback;\r\n" +
    "            xmlhttp.open(\"GET\", \"Switch\" , true);\r\n" +
    "            xmlhttp.send(null);\r\n" +
    "        }\r\n" +
    "        translateCallback = function (data) {\r\n" +
    "            if (data.status === 0) {\r\n" +
    "                var marker = new BMap.Marker(data.points[0]);\r\n" +
    "                bm.clearOverlays();\r\n" +
    "                bm.addOverlay(marker);\r\n" +
    "            }\r\n" +
    "            if(!firsttime){\r\n" +
    "                bm.setCenter(data.points[0]);\r\n" +
    "                bm.setZoom(19);\r\n" + 
    "                firsttime=1;\r\n" +
    "            }\r\n" +
    "        }\r\n" +
    "        var initPoint = new BMap.Point(118.578054, 24.810421);\r\n" +
    "        var bm = new BMap.Map(\"allmap\");\r\n" +
    "        var marker = new BMap.Marker(new BMap.Point(118.578054, 24.810421));\r\n" +
    "        bm.centerAndZoom(initPoint, 12);\r\n" +
    "        bm.addControl(new BMap.NavigationControl());\r\n" +
    "        function update_marker() {\r\n" +
    "            var x = parseFloat(str[0]), y = parseFloat(str[1]);\r\n" +
    "            if (x) {\r\n" +
    "                var ggPoint = new BMap.Point(x, y);\r\n" +
    "                var markergg = new BMap.Marker(ggPoint);\r\n" +
    "                var convertor = new BMap.Convertor();\r\n" +
    "                var pointArr = [];\r\n" +
    "                pointArr.push(ggPoint);\r\n" +
    "                convertor.translate(pointArr, 1, 5, translateCallback);\r\n" +
    "                document.getElementById(\"p2\").innerHTML = \"&nbsp&nbsp&nbsp&nbsp&nbsp经度：\" + str[0];\r\n" +
    "                document.getElementById(\"p1\").innerHTML = \"&nbsp&nbsp&nbsp&nbsp&nbsp纬度：\" + str[1];\r\n" +
    "            }\r\n" +
    "        }\r\n" +
    "    </script>\r\n" +
    "</body>\r\n" +
    "</html>";

void ICACHE_RAM_ATTR onTimerISR()
{
    // delay(500);
    //获取经纬度
    Serial.print("int= ");
    while (swSer.available() > 0)
    {
        gps.encode(swSer.read());
        if (gps.location.isUpdated())
        {
            Serial.print("Latitude= ");
            Serial.print(gps.location.lat(), 6);
            LAT = gps.location.lat();
            Serial.print(" Longitude= ");
            Serial.println(gps.location.lng(), 6);
            LNG = gps.location.lng();
        }
    }
    timer1_write(156250); //10ms
}

void setup()
{
    swSer.begin(GPSBaud);
    Serial.begin(115200);
    while (!Serial)
        ;
    // Connect to Wi-Fi network with SSID and password
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print("wait...");
    }
    // Print local IP address and start web server
    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
    timer1_attachInterrupt(onTimerISR);
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
    timer1_write(156250); //10ms
}

void loop()
{
    WiFiClient client = server.available(); //尝试建立客户对象
    if (client)                             //如果当前有客户可用
    {
        boolean currentLineIsBlank = true;
        Serial.println("[Client connected]");

        while (client.connected()) //如果客户端建立连接
        {
            if (client.available()) //等待有可读数据
            {
                char c = client.read(); //读取一字节数据
                readString += c;        //拼接数据
                /************************************************/
                if (c == '\n' && currentLineIsBlank) //等待请求头接收完成(接收到空行)
                {
                    //比较接收到的请求数据
                    if (readString.startsWith("GET / HTTP/1.1")) //如果是网页请求
                    {
                        client.print(responseHeaders); //向客户端输出网页响应
                        client.print(myhtmlPage);      //向客户端输出网页内容
                        client.print("\r\n");
                    }
                    else if (readString.startsWith("GET /Switch")) //如果是获取GPS数据的请求
                    {
                        sprintf(out_str,"%.6f,%.6f",LNG,LAT);
                        client.print(out_str);
                    }
                    else
                    {
                        client.print("\r\n");
                    }
                    break;
                }

                if (c == '\n')
                {
                    currentLineIsBlank = true; //开始新行
                }
                else if (c != '\r')
                {
                    currentLineIsBlank = false; //正在接收某行中
                }
                /************************************************/
            }
        }
        delay(1);      //等待客户完成接收
        client.stop(); //结束当前连接:
        Serial.println("[Client disconnected]");

        Serial.println(readString); //打印输出来自客户的数据
        readString = "";
    }
}