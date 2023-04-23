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

// constants
const CMD_PIDKP = "pid_kp";
const CMD_PIDKI = "pid_ki";
const CMD_PIDKD = "pid_kd";
const CMD_ERR = "err";
const CMD_TOGGLE = "toggle";
const CMD_MAXPOS = "max_pos";
const CMD_MAXVEL = "max_vel";
const CMD_PREVERR = "prev_err";
const CMD_LEDSTATE = "led_state";
const CMD_TARGETPOS = "target_pos";
const CMD_TARGETVEL = "target_vel";
const CMD_CURRENTPOS = "current_pos";
const CMD_CURRENTVEL = "current_vel";

// variables
let btn_onoff: HTMLButtonElement;                   // Toggle button to turn the robot on and off
let cvs_onoff: HTMLCanvasElement;                   // Canvas to show robot state 
let ctx_onoff: CanvasRenderingContext2D;            // Context to draw robot on off state on
// let btn_set_kx: HTMLButtonElement[] = [];           // Array of Buttons that When clicked submits kx value to robot
// let inp_kx: HTMLInputElement[] = [];                // Array of TextAreas where user can enter kx value

let inp_kp: HTMLInputElement[] = [];                // Array of Inputs to change and display motor kp values
let inp_ki: HTMLInputElement[] = [];                // Array of Inputs to change and display motor ki values
let inp_kd: HTMLInputElement[] = [];                // Array of Inputs to change and display motor kd values
let inp_pos: HTMLInputElement[] = [];               // Array of Inputs to change and display motor positions
let inp_vel: HTMLInputElement[] = [];               // Array of Inputs to change and display motor velocities
let inp_target_pos: HTMLInputElement[] = [];        // Array of Inputs to change and display motor pid target values
let inp_target_vel: HTMLInputElement[] = [];        // Array of Inputs to change and display motor pid target values
let inp_err: HTMLInputElement[] = [];               // Array of Inputs to display motor pid errors
let inp_prev_err: HTMLInputElement[] = [];          // Array of Inputs to display previous errors
let inp_max_pos: HTMLInputElement[] = [];           // Array of Inputs to change and display max motor values
let inp_max_vel: HTMLInputElement[] = [];           // Array of Inputs to change and display max motor values


/**
    * Returns the input if it is not null or undefined.
    * Throws an error if input is null or undefined.
    * @param element - The element to verify.
    * @returns The input if it is not null or undefined.
    */
function verifyElement(element: HTMLElement | null | undefined): HTMLElement {
    if (element !== null && element !== undefined) {
        return element;
    }
    throw new Error('Element cannot be null or undefined');
}


/**
 * Determines whether the given variable is a finite number.
 * 
 * @param variable - The variable to test.
 * @returns True if the variable is a finite number, false otherwise.
 */
function isNumber(variable: any): variable is number {
    return typeof variable === 'number' && isFinite(variable);
}


// This is called when the page finishes loading
function init(): void {

    // Assign page elements to variables
    btn_onoff = <HTMLButtonElement>verifyElement(document.getElementById("btn_onoff"));
    btn_onoff.onclick = onclick_btn_onoff;
    btn_onoff.innerHTML = "Turn on";

    cvs_onoff = <HTMLCanvasElement>verifyElement(document.getElementById("cvs_onoff"));
    if (cvs_onoff == null) {
        return;
    }
    cvs_onoff.width = 50;
    cvs_onoff.height = 50;

    // const parameters = ["kp", "ki", "kd"];
    // for (let i = 0; i < parameters.length; i++) {
    //     let kx: string = parameters[i];
    //     for (let j = 0; j < 4; j++) {
    //         inp_kx[i*j] = <HTMLInputElement>verifyElement(document.getElementById(`inp_${kx}_${j}`));
    //         inp_kx[i*j].value = "0";
    //         inp_kx[i*j].onchange = function (event) { console.log(event); onsubmit_inp_set_kx(kx, j); }

    //     }

    //     // inp_kx[v] = <HTMLInputElement>verifyElement(document.getElementById(`inp_${v}`));
    //     // inp_kx[v].value = "0";
    //     // inp_kx[v].innerHTML = "0";

    //     // btn_set_kx[v] = <HTMLButtonElement>verifyElement(document.getElementById(`btn_set_${v}`));
    //     // btn_set_kx[v].disabled = false;
    //     // btn_set_kx[v].onclick = function (event: Event) { console.log(event); onclick_btn_set_kx(v); }
    // }


    for (let i = 0; i < 3; i++) {
        inp_pos[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_pos_${i}`));
        inp_pos[i].value = "0";

        inp_vel[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_vel_${i}`));
        inp_vel[i].value = "0";

    }

    for (let i = 0; i < 8; i++) {
        inp_kp[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_kp_${i}`));
        inp_kp[i].value = "0";
        inp_kp[i].onchange = function (event) { console.log(event); onsubmit_inp_set_kp(i); }

        inp_ki[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_ki_${i}`));
        inp_ki[i].value = "0";
        inp_ki[i].onchange = function (event) { console.log(event); onsubmit_inp_set_ki(i); }

        inp_kd[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_kd_${i}`));
        inp_kd[i].value = "0";
        inp_kd[i].onchange = function (event) { console.log(event); onsubmit_inp_set_kd(i); }
    }


    for (let i = 0; i < 6; i++) {
        inp_err[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_err_${i}`));
        inp_err[i].value = "0";

        inp_prev_err[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_prev_err_${i}`));
        inp_prev_err[i].value = "0";
    }


    for (let i = 0; i < 4; i++) {
        inp_target_pos[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_target_pos_${i}`));
        inp_target_pos[i].value = "0";
        inp_target_pos[i].onchange = function (event) { console.log(event); onsubmit_inp_set_target_pos(i); }

        inp_target_vel[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_target_vel_${i}`));
        inp_target_vel[i].value = "0";
        inp_target_vel[i].onchange = function (event) { console.log(event); onsubmit_inp_set_target_vel(i); }


        inp_max_pos[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_max_pos_${i}`));
        inp_max_pos[i].value = "0";
        inp_max_pos[i].onchange = function (event) { console.log(event); onsubmit_inp_set_max_pos(i); }

        inp_max_vel[i] = <HTMLInputElement>verifyElement(document.getElementById(`inp_max_vel_${i}`));
        inp_max_vel[i].value = "0";
        inp_max_vel[i].onchange = function (event) { console.log(event); onsubmit_inp_set_max_vel(i); }
    }


    // Draw circle in cvs_onoff
    ctx_onoff = cvs_onoff.getContext("2d")!;
    ctx_onoff.arc(25, 25, 15, 0, Math.PI * 2, false);
    ctx_onoff.lineWidth = 3;
    ctx_onoff.strokeStyle = "black";
    ctx_onoff.stroke();
    turn_off();

    // Connect to WebSocket server
    let url = get_url();
    wsConnect(url);
}


function get_url(): string {
    return "ws://" + window.location.hostname + ":1337/";
}


function turn_off(): void {
    console.log("LED is off");
    ctx_onoff.fillStyle = "black";
    ctx_onoff.fill();
    btn_onoff.innerHTML = "Turn on";
}


function turn_on(): void {
    console.log("LED is on");
    ctx_onoff.fillStyle = "red";
    ctx_onoff.fill();
    btn_onoff.innerHTML = "Turn off";
}


// Called whenever the HTML button is pressed
function onclick_btn_onoff() {
    doSend(CMD_TOGGLE);
}


// Called whenever a set kp, ki or kd button is pressed    
// function onclick_btn_set_kx(parm): void {
//     console.log("onclick: " + parm)
//     let value = inp_kx[parm].value;
//     let cmd = `pid_${parm}:${value}`;
//     doSend(cmd);
// }


function onsubmit_inp_set_target_pos(index: number): void {
    console.log(`onsubmit: target from ${index}`);
    var value = inp_target_pos[index].value;
    var cmd = `${CMD_TARGETPOS}:${index};${value},`;
    doSend(cmd);
}


function onsubmit_inp_set_target_vel(index: number): void {
    console.log(`onsubmit: target from ${index}`);
    var value = inp_target_vel[index].value;
    var cmd = `${CMD_TARGETVEL}:${index};${value},`;
    doSend(cmd);
}


function onsubmit_inp_set_max_pos(index: number): void {
    console.log(`onsubmit: max value from ${index}`);
    var value = inp_max_pos[index].value;
    var cmd = `${CMD_MAXPOS}:${index};${value},`;
    doSend(cmd);
}


function onsubmit_inp_set_max_vel(index: number): void {
    console.log(`onsubmit: max value from ${index}`);
    var value = inp_max_vel[index].value;
    var cmd = `${CMD_MAXVEL}:${index};${value},`;
    doSend(cmd);
}


function onsubmit_inp_set_kp(index: number): void {
    console.log(`onsubmit: kp from ${index}`);
    var value = inp_kp[index].value;
    var cmd = `${CMD_PIDKP}:${index};${value},`;
    doSend(cmd);
}


function onsubmit_inp_set_ki(index: number): void {
    console.log(`onsubmit: kp from ${index}`);
    var value = inp_ki[index].value;
    var cmd = `${CMD_PIDKI}:${index};${value},`;
    doSend(cmd);
}


function onsubmit_inp_set_kd(index: number): void {
    console.log(`onsubmit: kp from ${index}`);
    var value = inp_kd[index].value;
    var cmd = `${CMD_PIDKD}:${index};${value},`;
    doSend(cmd);
}


// function onsubmit_inp_set_kx(kx: string, index: number): void {
//     console.log(`onsubmit: ${kx} from ${index}`);
//     var value = inp_kx
//     var cmd = `pid_kp:${index};${value},`;
//     doSend(cmd);
// }


function get_data(event: MessageEvent, last_split: number): string[] {
    let values = [];
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
    return values;
}


function request_all_data(): void {
    doSend(CMD_LEDSTATE + ":?");
    doSend(CMD_PIDKP + ":?");
    doSend(CMD_PIDKI + ":?");
    doSend(CMD_PIDKD + ":?");
    doSend(CMD_CURRENTPOS + ":?");
    doSend(CMD_CURRENTVEL + ":?");
    doSend(CMD_ERR + ":?");
    doSend(CMD_PREVERR + ":?");
    doSend(CMD_MAXPOS + ":?");
    doSend(CMD_MAXVEL + ":?");
    doSend(CMD_TARGETPOS + ":?");
    doSend(CMD_TARGETVEL + ":?");
}


/* 
    * Web sockets section          
*/

// Call this to connect to the WebSocket server
let websocket: WebSocket;
function wsConnect(url: string | URL): WebSocket {
    // Connect to WebSocket server
    websocket = new WebSocket(url);

    // Assign callbacks
    websocket.onopen = function (event) { onOpen(event) };
    websocket.onclose = function (event) { onClose(event) };
    websocket.onmessage = function (event) { onMessage(event) };
    websocket.onerror = function (event) { onError(event) };
    return websocket;
}

// Called when a WebSocket connection is established with the server
function onOpen(event: Event) {

    // Log connection state
    console.log(`Connected: ${event}`);

    // Enable controls
    btn_onoff.disabled = false;

    // Get the current state of gui
    request_all_data();

    // bad that all clients schedules a request every x ms but it works for few clients
    // setInterval(function () { request_all_data(); }, 2000);
}

// Called when the WebSocket connection is closed
function onClose(event: CloseEvent) {

    // Log disconnection state
    console.log(`Disconnected: ${event.code} (${event.reason})`);

    // Disable controls
    // btn_onoff.disabled = true;

    // Try to reconnect after a few seconds
    setTimeout(function () { let url = get_url(); wsConnect(url) }, 2000);
}

// Called when a message is received from the server
function onMessage(event: MessageEvent): void {
    let command, last_split; // value

    // Print out our received message
    console.log("Received: " + event.data);

    let command_marker = event.data.search(':')

    if (command_marker == -1) { // no : in data
        // command = event.data;
        // value = "";
        console.log("no : in data");
        return;
    }

    command = event.data.slice(0, command_marker);
    // console.log("command: ", command);

    let id = null;
    let id_marker = event.data.search(';');
    if (id_marker == -1) {
        // value = event.data.slice(command_marker + 1);
        last_split = command_marker;
    } else {
        // value = event.data.slice(id_marker + 1);
        last_split = id_marker;
        id = parseInt(event.data.slice(command_marker + 1, id_marker));
        console.log("id ${}: ${}", typeof id, id);
    }

    let values = get_data(event, last_split);
    console.log(`${command} data:`);
    console.log(`values array has size: ${values.length} and contains: ${values}`);

    // Update circle graphic with LED state
    switch (command) {
        case "0":
            turn_off();
            break;
        case "1":
            turn_on();
            break;
        case CMD_LEDSTATE:
            if (last_split + 1 == '1') {
                turn_on()
            } else {
                turn_off()
            }
            break;
        case CMD_PIDKP:
            if (!isNumber(id)) {
                for (let i = 0; i < inp_kp.length - 2; i++) {
                    console.log(`${command} values received: ${values[i]}`);
                    inp_kp[i].value = values[i];
                }
                return;
            }
            switch (id) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                    {
                        for (let value of values) {
                            inp_kp[id].value = value;
                        }
                    }
                    break;
                case 6:
                    {
                        let index: number = 0;
                        for (let value of values) {
                            inp_kp[index].value = value;
                            index += 2;
                        }
                    }
                    break;
                case 7:
                    {
                        let index: number = 1;
                        for (let value of values) {
                            inp_kp[index].value = value;
                            index += 2;
                        }
                    }
                    break;
                default:
                    console.log("invalid id: ", id);
                    break;
            }
            break;
        case CMD_PIDKI:
            {
                if (!isNumber(id)) {
                    for (let i = 0; i < inp_ki.length - 2; i++) {
                        console.log(`${command} values received: ${values[i]}`);
                        inp_ki[i].value = values[i];
                    }
                    return;
                }
                switch (id) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                        {
                            for (let value of values) {
                                inp_ki[id].value = value;
                            }
                        }
                        break;
                    case 6:
                        {
                            let index: number = 0;
                            for (let value of values) {
                                inp_ki[index].value = value;
                                index += 2;
                            }
                        }
                        break;
                    case 7:
                        {
                            let index: number = 1;
                            for (let value of values) {
                                inp_ki[index].value = value;
                                index += 2;
                            }
                        }
                        break;
                    default:
                        console.log("invalid id: ", id);
                        break;
                }
                break;
            }
        case CMD_PIDKD:
            {
                if (!isNumber(id)) {
                    for (let i = 0; i < inp_kd.length - 2; i++) {
                        console.log(`${command} values received: ${values[i]}`);
                        inp_kd[i].value = values[i];
                    }
                    return;
                }
                switch (id) {
                    case 0:
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                        {
                            for (let value of values) {
                                inp_kd[id].value = value;
                            }
                        }
                        break;
                    case 6:
                        {
                            let index: number = 0;
                            for (let value of values) {
                                inp_kd[index].value = value;
                                index += 2;
                            }
                        }
                        break;
                    case 7:
                        {
                            let index: number = 1;
                            for (let value of values) {
                                inp_kd[index].value = value;
                                index += 2;
                            }
                        }
                        break;
                    default:
                        console.log("invalid id: ", id);
                        break;
                }
                break;
            }
        case CMD_CURRENTPOS:
            {
                for (let i = 0; i < inp_pos.length - 1; i++) {
                    console.log(`position values received: ${values[i]}`);
                    inp_pos[i].value = values[i];
                }
                break;
            }
        case CMD_CURRENTVEL:
            {
                for (let i = 0; i < inp_vel.length - 1; i++) {
                    console.log(`velocity values received: ${values[i]}`);
                    inp_vel[i].value = values[i];
                }
                break;
            }
        case CMD_ERR:
            {
                for (let i = 0; i < inp_err.length - 1; i++) {
                    console.log(`${command} values received: ${values[i]}`);
                    inp_err[i].value = values[i];
                }
                break;
            }
        case CMD_PREVERR:
            {
                for (let i = 0; i < inp_prev_err.length - 1; i++) {
                    console.log(`${command} values received: ${values[i]}`);
                    inp_prev_err[i].value = values[i];
                }
                break;
            }
        case CMD_MAXPOS:
            {
                if (!isNumber(id)) {
                    for (let i = 0; i < inp_max_pos.length - 1; i++) {
                        console.log(`max_pos values received: ${values[i]}`);
                        inp_max_pos[i].value = values[i];
                    }
                    return;
                }
                switch (id) {
                    case 0:
                    case 1:
                    case 2:
                        {
                            for (let value of values) {
                                inp_max_pos[id].value = value;
                            }
                        }
                        break;
                    case 3:
                        {
                            for (let index = 0; index < inp_max_pos.length - 1; index++) {
                                inp_max_pos[index].value = values[index];
                            }
                        }
                        break;
                    default:
                        console.log("invalid id: ", id);
                        break;
                }
                break;
            }
        case CMD_MAXVEL:
            {
                if (!isNumber(id)) {
                    for (let i = 0; i < inp_max_vel.length - 1; i++) {
                        console.log(`${command} values received: ${values[i]}`);
                        inp_max_vel[i].value = values[i];
                    }
                    return;
                }
                switch (id) {
                    case 0:
                    case 1:
                    case 2:
                        {
                            for (let value of values) {
                                inp_max_vel[id].value = value;
                            }
                        }
                        break;
                    case 3:
                        {
                            for (let index = 0; index < inp_max_vel.length - 1; index++) {
                                inp_max_vel[index].value = values[index];
                            }
                        }
                        break;
                    default:
                        console.log("invalid id: ", id);
                        break;
                }
                break;
            }
        case CMD_TARGETPOS:
            {
                if (!isNumber(id)) {
                    for (let i = 0; i < inp_target_pos.length - 1; i++) {
                        console.log(`${command} values received: ${values[i]}`);
                        inp_target_pos[i].value = values[i];
                    }
                    return;
                }
                switch (id) {
                    case 0:
                    case 1:
                    case 2:
                        {
                            for (let value of values) {
                                inp_target_pos[id].value = value;
                            }
                        }
                        break;
                    case 3:
                        {
                            for (let index = 0; index < inp_target_pos.length - 1; index++) {
                                inp_target_pos[index].value = values[index];
                            }
                        }
                        break;
                    default:
                        console.log("invalid id: ", id);
                        break;
                }
                break;
            }
        case CMD_TARGETVEL:
            {
                if (!isNumber(id)) {
                    for (let i = 0; i < inp_target_vel.length - 1; i++) {
                        console.log(`${command} values received: ${values[i]}`);
                        inp_target_vel[i].value = values[i];
                    }
                    return;
                }
                switch (id) {
                    case 0:
                    case 1:
                    case 2:
                        {
                            for (let value of values) {
                                inp_target_vel[id].value = value;
                            }
                        }
                        break;
                    case 3:
                        {
                            for (let index = 0; index < inp_target_vel.length - 1; index++) {
                                inp_target_vel[index].value = values[index];
                            }
                        }
                        break;
                    default:
                        console.log("invalid id: ", id);
                        break;
                }
                break;
            }
        default:
            break;
    }
}

// Called when a WebSocket error occurs
function onError(event: Event) {
    console.log("ERROR: " + event);
}

// Sends a message to the server (and prints it to the console)
function doSend(message: string) {
    console.log("Sending: " + message);
    websocket.send(message);
}

// Call the init function as soon as the page loads
window.addEventListener("load", init, false);

