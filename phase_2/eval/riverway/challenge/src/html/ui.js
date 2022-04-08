
    const indicator_statuses = [ "indicator-off", "indicator-on", "indicator-error"];

    function runSim() {

        velocity_chart.clear_data();
        altitude_chart.clear_data();

        disableControls();
        socket.send(JSON.stringify({"cmd":"start"}));
    }

    function showSCConfig() {

        socket.send(JSON.stringify({"cmd":"getconfig"}));

        document.getElementById("formly1").style.display = "block";

    }

    function showSimConfig() {

        socket.send(JSON.stringify({"cmd":"getsimconfig"}));

        document.getElementById("formly2").style.display = "block";

    }

    function sendLog() {

        let logname = document.getElementById("simlogs").value;

        socket.send(JSON.stringify({"cmd":"sendlog", "name": logname}));

        document.getElementById("logs").style.display = "none";

    }

    function showLogs() {

        socket.send(JSON.stringify({"cmd":"listlogs"}));

        document.getElementById("logs").style.display = "block";

    }

    function closeLogs() {

        document.getElementById("logs").style.display = "none";

    }

    function closeForm() {

        document.getElementById("formly1").style.display = "none";

    }
    function closeSimForm() {

        document.getElementById("formly2").style.display = "none";

    }

    function sendConfig() {


        document.getElementById("formly1").style.display = "none";


        let kp = Number(document.getElementById("Kp").value);

        if (Number.isNaN(kp)) {

            alert("Bad value for Kp");
            return;

        }

        
        let ki = Number(document.getElementById("Ki").value);

        if (Number.isNaN(ki)) {

            alert("Bad value for ki");
            return;

        }

        let kd = Number(document.getElementById("Kd").value);

        if (Number.isNaN(kd)) {

            alert("Bad value for kd");
            return;

        }

        let ivelocity = Number(document.getElementById("ivelocity").value);

        if (Number.isNaN(ivelocity)) {data

            alert("Bad value for ivelocity");
            return;

        }

        let fvelocity = Number(document.getElementById("fvelocity").value);

        if (Number.isNaN(fvelocity)) {

            alert("Bad value for fvelocity");
            return;

        }
        let ialtitude = Number(document.getElementById("ialtitude").value);

        if (Number.isNaN(ialtitude)) {

            alert("Bad value for ialtitude");
            return;

        }
        let faltitude = Number(document.getElementById("faltitude").value);

        if (Number.isNaN(faltitude)) {

            alert("Bad value for faltitude");
            return;

        }
        let fuel_max = Number(document.getElementById("fuel_max").value);

        if (Number.isNaN(fuel_max)) {

            alert("Bad value for sim_alt");
            return;

        }
        let thrust_max = Number(document.getElementById("thrust_max").value);

        if (Number.isNaN(thrust_max)) {

            alert("Bad value for sim_vel");
            return;

        }

        // let planet = document.getElementById("planets").value;

        socket.send(JSON.stringify({"cmd":"setconfig",
                                    "Kp":kp,
                                    "Ki":ki,
                                    "Kd":kd,
                                    "ivelocity": ivelocity,
                                    "fvelocity": fvelocity,
                                    "ialtitude": ialtitude,
                                    "faltitude": faltitude,
                                    "fuel_max": fuel_max,
                                    "thrust_max": thrust_max,
                                    "mpg" : mpg,
                                  })
        );


    }

    function sendSimConfig() {


        document.getElementById("formly2").style.display = "none";

        let sim_alt = Number(document.getElementById("sim_alt").value);

        if (Number.isNaN(sim_alt)) {

            alert("Bad value for sim_alt");
            return;

        }
        let sim_vel = Number(document.getElementById("sim_vel").value);

        if (Number.isNaN(sim_vel)) {

            alert("Bad value for sim_vel");
            return;

        }

        let planet = document.getElementById("planets").value;

        let logging = document.getElementById("logging").checked;

        socket.send(JSON.stringify({"cmd":"setsimconfig",
                                    "sim_alt": sim_alt,
                                    "sim_vel": sim_vel,
                                    "planet": planet,
                                    "logging": logging,
                                })
        );


    }

    function disableControls() {

        document.getElementById("run-btn").disabled = true;
        document.getElementById("change-btn").disabled = true;
        document.getElementById("logs-btn").disabled = true;
        document.getElementById("simconfig-btn").disabled = true;
    }

    function enableControls() {

        document.getElementById("run-btn").disabled = false;
        document.getElementById("change-btn").disabled = false;
        document.getElementById("logs-btn").disabled = false;
        document.getElementById("simconfig-btn").disabled = false;

    }

    document.getElementById("parachute").className = "indicator-on";

    var fuel_canvas = document.getElementById("fuel");

    fuel_bar = new Bar(fuel_canvas, 100, "Fuel%");

    fuel_bar.update_value(0);

    var thrust_canvas = document.getElementById("thrust");

    thrust_bar = new Bar(thrust_canvas, 100, "Thrust%");

    thrust_bar.update_value(0);

    var velocitybar_canvas = document.getElementById("velocitybar");

    velocity_bar = new Bar(velocitybar_canvas, 500, "Velocity (m/s)");

    velocity_bar.update_value(0);

    var altitudebar_canvas = document.getElementById("altitudebar");

    altitude_bar = new Bar(altitudebar_canvas, 9300, "Altitude (m)");

    altitude_bar.update_value(0);

    log = document.getElementById("log");
    vel_chart = document.getElementById("velocity");

    alt_chart = document.getElementById("altitude");

    velocity_chart = new Chart(vel_chart, 120, 500, "Time Delta", "Velocity (m/s)");
    altitude_chart = new Chart(alt_chart, 120, 9300, "Time Delta", "Altitude (m)");

    velocity_chart.draw_graph();
    altitude_chart.draw_graph();


    var url = window.location.href;
    var websocket_addr = "ws://" + url.split('/')[2];
    let socket = new WebSocket(websocket_addr);


    socket.onopen = function(event) {

        console.log("websocket opened");

    }

    socket.onclose = function(event) {

        disableControls();

      }

    socket.onmessage = function(event) {
    
      var message = JSON.parse(event.data);

      switch(message['type']) {

        case 'data':

            position = parseFloat(message['altitude']);
            velocity = parseFloat(message['velocity']);
            fuel = parseInt(message['fuel%']);
            thrust = parseInt(message['thrust%']);
            state = parseInt(message['spacecraft_state'], 16);
            delta_time = parseFloat(message['sim_time_delta']);

            touchdown = (state >> 11) & 3;
            parachute = (state >> 9) & 3;
            legs_deployed = (state >> 7) & 3;
            backshell_ejected = (state >> 5) & 3;
            constant_velocity = (state >> 3 ) & 3;
            engine_cutoff = (state >> 1) & 3;
            powered_descent = state & 1;

            document.getElementById("legs_deployed").className = indicator_statuses[legs_deployed];
            document.getElementById("backshell").className = indicator_statuses[backshell_ejected];
            document.getElementById("powered_descent").className = indicator_statuses[powered_descent];
            document.getElementById("constant_velocity").className = indicator_statuses[constant_velocity];
            document.getElementById("touchdown").className = indicator_statuses[touchdown];
            document.getElementById("engine_cutoff").className = indicator_statuses[engine_cutoff];

            velocity_chart.add_datapoint(delta_time, Math.abs(velocity));
            velocity_chart.draw_graph();

            altitude_chart.add_datapoint(delta_time, position);
            altitude_chart.draw_graph();

            fuel_bar.update_value(fuel);
            thrust_bar.update_value(thrust);

            altitude_bar.update_value(position.toFixed(1));
            velocity_bar.update_value(Math.abs(velocity.toFixed(1)));
            
            break;

        case 'status':

            enableControls();

            alert(message['message']);

            break;

        case 'config':

            document.getElementById("Kp").value = message["Kp"];
            document.getElementById("Ki").value = message["Ki"];
            document.getElementById("Kd").value = message["Kd"];
            document.getElementById("ivelocity").value = message["ivelocity"];
            document.getElementById("fvelocity").value = message["fvelocity"];
            document.getElementById("ialtitude").value = message["ialtitude"];
            document.getElementById("faltitude").value = message["faltitude"];
            document.getElementById("fuel_max").value = message['fuel_max'];
            document.getElementById("thrust_max").value = message['thrust_max'];
            document.getElementById("mpg").value = message['mpg'];

            break;

        case 'simconfig':

            document.getElementById("sim_alt").value = message["sim_alt"];
            document.getElementById("sim_vel").value = message["sim_vel"];
            let planets = message["planets"];

            select = document.getElementById("planets");

            select.options.length = 0;

            for (i=0; i < planets.length; ++i) {
                var opt = document.createElement('option');
                opt.value = planets[i];
                opt.innerHTML = planets[i];
                select.appendChild(opt);

            }

            select.value = message['selected_planet'];

        case 'loglist':

            let loglist = message["logs"];

            select = document.getElementById("simlogs");

            select.options.length = 0;

            for (i=0; i < loglist.length; ++i) {
                var opt = document.createElement('option');
                opt.value = loglist[i];
                opt.innerHTML = loglist[i];
                select.appendChild(opt);

            }
            break;

        case 'logdata':


            let logWin = window.open("about:blank", "LogData", "width=400,height=400");
            logWin.document.write('<pre>');
            logWin.document.write(message['data']);
            logWin.document.write('</pre>');
            break;

      }


    
    };
    