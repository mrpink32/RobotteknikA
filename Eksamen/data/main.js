/*
         *  Coding guidelines (rules that ensures fast support from teacher):
         *
         *  1. Use <right-click><Format Document> often and always before upload 
         *
         *  2. Use same global variable name in js as used in html id property
         *
         *  3. Use the following naming convention for global js variables (all letters lower case):
         * 
         *      button:     btn_xxx
         *      canvas:     cvs_xxx
         *      input:      inp_xxx
         *      textarea:   txa_xxx
         *      p:          par_xxx
         *      context:    ctx_xxx
         *      ... to be continued ...
         *
         *  4. Comment on all global variables in js
         *
         *  5. Event callback functions that are only used once must follow the following naming convention:
         *      
         *      objectvariable.event = event_objectvariable     
         * 
         *      Example:  btn_onoff.onclick = onclick_btn_onoff
         * 
         *  6. Set properties in javascript on elements retrieved by getElementById(...) instead of setting properties in html body section
         * 
         *  7. Avoid using setInterval(...) for polling if possible
         * 
         *  8. websockets commands follows the syntax:
         *          
         *          <websocket_cmd>:   "<cmd_list> | <cmd>"
         *          <cmd_list>:        <cmd>;<cmd>;...       
         *          <cmd>:             <identier> | <identifier>:<parameter>
         *          <identifier>:      <name> | <subsystem>_<name>
         *          <name>:            <txt>  
         *          <subsystem>:       <txt>
         *          <parameter>:       <txt> | <tal> | <request>
         *          <request>:         ?
         *          <txt>:             tekst streng der starter med et bogstab og efterfølges af en eller flere bogstaver, symboler eller tal
         *          <tal>:             tekst streng der indeholder et heltal eller kommatal der kan fortolkes af robotten
         *          
         *     Examples: 
         *          "on"                                ==> turn robot on 
         *          "off"                               ==> turn robot off 
         *          "state:on"                          ==> turn robot on or off, parameter is on|off
         *          "state:?"                           ==> request if robot is turned on or off   
         *          "pid_kp:2.5"                        ==> set the pid kp parameter to 2.5
         *          "pid_kp:?"                          ==> request the kp parameter
         *          "stat:off; pid_kp:2.1; stat:on"     ==> multiple commands at once: turn off, set kp=2.1, turn on 
         *      
         */

// constans
const Cmd_Toggle = "toggle";
const Cmd_LedState = "led_state";
const Cmd_Pos = "req_pos";
const Cmd_Vel = "req_vel";


// system vars
var btn_onoff;          // Toggle button to turn the robot on and off
var cvs_onoff;          // Canvas to show robot state 
var ctx_onoff;          // Context to draw robot on off state on
// var par_led_intensity;  // Paragraph that shows led intensity
// var inp_slider;         // Slider that sets pwm value and thus led intensity 
// pid subsystem vars 
var btn_set_kx = [];    // Array of Buttons that When clicked submits kx value to robot
var inp_kx = [];        // Array of TextAreas where user can enter kx value
var inp_pos = [];       // Array of Inputs to change and display motor positions
var inp_vel = [];       // Array of Inputs to change and display motor velocities


function get_url() {
    return "ws://" + window.location.hostname + ":1337/";
}

function turn_off() {
    console.log("LED is off");
    ctx_onoff.fillStyle = "black";
    ctx_onoff.fill();
    btn_onoff.innerHTML = "Turn on";
}

function turn_on() {
    console.log("LED is on");
    ctx_onoff.fillStyle = "red";
    ctx_onoff.fill();
    btn_onoff.innerHTML = "Turn off";
}

// This is called when the page finishes loading
function init() {

    // Assign page elements to variables
    btn_onoff = document.getElementById("btn_onoff");
    btn_onoff.onclick = onclick_btn_onoff;
    btn_onoff.innerHTML = "Turn on";

    cvs_onoff = document.getElementById("cvs_onoff");
    cvs_onoff.width = 50;
    cvs_onoff.height = 50;

    const parameters = ["kp", "ki", "kd"];
    for (const p in parameters) {
        let v = parameters[p] // important to use let

        inp_kx[v] = document.getElementById(`inp_${v}`);
        inp_kx[v].value = "0";
        inp_kx[v].innerHTML = "0";
        btn_set_kx[v] = document.getElementById(`btn_set_${v}`);
        btn_set_kx[v].disabled = false;
        btn_set_kx[v].onclick = function (event) { console.log(event); onclick_btn_set_kx(v); }
    }

    for (let i = 0; i < 4; i++) {
        inp_pos[i] = document.getElementById(`inp_pos_${i + 1}`);
        inp_pos[i].value = "0";
        inp_pos[i].onchange = function (event) { console.log(event); onsubmit_inp_set_pos(i); }
    }

    for (let i = 1; i < 5; i++) {
        inp_vel[i] = document.getElementById(`inp_vel_${i}`);
        inp_vel[i].value = "0";
        inp_vel[i].onchange = function (event) { console.log(event); onsubmit_inp_set_vel(i); }
    }

    // Draw circle in cvs_onoff
    ctx_onoff = cvs_onoff.getContext("2d");
    ctx_onoff.arc(25, 25, 15, 0, Math.PI * 2, false);
    ctx_onoff.lineWidth = 3;
    ctx_onoff.strokeStyle = "black";
    ctx_onoff.stroke();
    turn_off();

    // Connect to WebSocket server
    url = get_url();
    wsConnect(url);
}

// Called whenever the HTML button is pressed
function onclick_btn_onoff() {
    doSend(Cmd_Toggle);
}

// Called whenever a set kp, ki or kd button is pressed    
function onclick_btn_set_kx(parm) {
    console.log("onclick: " + parm)
    var value = inp_kx[parm].value;
    var cmd = `pid_${parm}:${value}`;
    doSend(cmd);
}

function onsubmit_inp_set_pos(index) {
    console.log("onsubmit: position from `${index}`")
    var value = inp_pos[index].value;
    var cmd = `req_pos:${value},${index},`;
    doSend(cmd);
}

function onsubmit_inp_set_vel(index) {
    console.log("onclick:  grim")
    var value = inp_vel[index].value;
    var cmd = `req_vel:${value}`;
    doSend(cmd);
}

function request_all_data() {
    doSend(Cmd_LedState + ":?");
    doSend("pid_kp:?");
    doSend("pid_ki:?");
    doSend("pid_kd:?");
    doSend(Cmd_Pos + ":?");
    doSend(Cmd_Vel + ":?");
}


/* 
 * Web sockets section          
 */

// Call this to connect to the WebSocket server
function wsConnect(url) {
    // Connect to WebSocket server
    websocket = new WebSocket(url);

    // Assign callbacks
    websocket.onopen = function (event) { onOpen(event) };
    websocket.onclose = function (event) { onClose(event) };
    websocket.onmessage = function (event) { onMessage(event) };
    websocket.onerror = function (event) { onError(event) };
}

// Called when a WebSocket connection is established with the server
function onOpen(event) {

    // Log connection state
    console.log("Connected");

    // Enable controls
    btn_onoff.disabled = false;
    btn_set_kp.disabled = false;
    btn_set_ki.disabled = false;
    btn_set_kd.disabled = false;

    // Get the current state of gui
    request_all_data();

    // bad that all clients schedules a request every x ms but it works for few clients
    // setInterval(function () { request_all_data(); }, 2000);
}

// Called when the WebSocket connection is closed
function onClose(event) {

    // Log disconnection state
    console.log("Disconnected");

    // Disable controls
    btn_onoff.disabled = true;
    btn_set_kp.disabled = true;
    btn_set_ki.disabled = true;
    btn_set_kd.disabled = true;

    // Try to reconnect after a few seconds
    setTimeout(function () { url = get_url(); wsConnect(url) }, 2000);
}

// Called when a message is received from the server
function onMessage(event) {

    // Print out our received message
    console.log("Received: " + event.data);

    let idx = event.data.search(':')

    let command, value;
    var values = [];

    if (idx == -1) { // no : in data
        command = event.data;
        value = "";
    } else {
        command = event.data.slice(0, idx);
        value = event.data.slice(idx + 1);

        if (command == Cmd_Pos || command == Cmd_Vel) {
            let last_split = idx;
            let split = 0;
            while (true) {
                split = event.data.indexOf(',', last_split + 1);
                // console.log("split: ", split)
                if (split == -1) {
                    break;
                }
                values.push(event.data.slice(last_split + 1, split));
                last_split = event.data.indexOf(',', split);
                // console.log("last_split: ", last_split);
            }
        }
    }

    // Update circle graphic with LED state
    switch (command) {
        case "0":
            turn_off();
            break;
        case "1":
            turn_on();
            break;
        case "led_state":
            if (value == '1') {
                turn_on()
            } else {
                turn_off()
            }
            break;
        case "pid_kp":
            console.log(`kp value received: ${value}`);
            inp_kx["kp"].value = value;
            break;
        case "pid_ki":
            console.log(`ki value received: ${value}`);
            inp_kx["ki"].value = value;
            break;
        case "pid_kd":
            console.log(`kd value received: ${value}`);
            inp_kx["kd"].value = value;
            break;
        case "req_pos": {
            console.log("Positions data:");
            let length = values.length;
            console.log(`values array has size: ${length} and contains: ${values}`)
            for (let i = 0; i < length; i++) {
                console.log(`position values received: ${values[i]}`);
                inp_pos[i].value = values[i];
            }
            break;
        }
        case "req_vel": {
            console.log("Velocity data:");
            let length = values.length;
            console.log(`values array has size: ${length} and contains: ${values}`)
            for (let i = 0; i < length; i++) {
                console.log(`velocity values received: ${values[i]}`);
                inp_pos[i].value = values[i];
            }
            break;
        }
        default:
            break;
    }
}

// Called when a WebSocket error occurs
function onError(event) {
    console.log("ERROR: " + event.data);
}

// Sends a message to the server (and prints it to the console)
function doSend(message) {
    console.log("Sending: " + message);
    websocket.send(message);
}

// Call the init function as soon as the page loads
window.addEventListener("load", init, false);
