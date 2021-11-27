const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>Settings</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
      .button {
        padding: 10px 20px;
        font-size: 24px;
        text-align: center;
        outline: none;
        color: #fff;
        background-color: #2f4468;
        border: none;
        border-radius: 5px;
        box-shadow: 0 6px #999;
        cursor: pointer;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        -webkit-tap-highlight-color: rgba(0,0,0,0);
      }  
      .button:hover {background-color: #1f2e45}
      .button:active {
        background-color: #1f2e45;
        box-shadow: 0 4px #666;
        transform: translateY(2px);
      }
    </style>
  </head>
  <body>
    <h1>Settings</h1>
    <button class="button" onmousedown="toggleCheckbox('up');" ontouchstart="toggleCheckbox('up');">Up</button>
    <button class="button" onmousedown="toggleCheckbox('down');" ontouchstart="toggleCheckbox('down');">Down</button>
    <button class="button" onmousedown="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('stop');">Stop</button>
    <button class="button" onmousedown="toggleCheckbox('save_up');" ontouchstart="toggleCheckbox('save_up');">Save Start Position</button>
    <button class="button" onmousedown="toggleCheckbox('save_down');" ontouchstart="toggleCheckbox('save_down');">Save Stop Position</button>
    <button class="button" onmousedown="toggleCheckbox('save_config');" ontouchstart="toggleCheckbox('save_config');">Save Config</button>
   <script>
   function toggleCheckbox(x) {
     var xhr = new XMLHttpRequest();
     xhr.open("GET", "/" + x, true);
     xhr.send();
   }
  </script>
  </body>
</html>)rawliteral";
