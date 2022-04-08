const { lstat, write } = require('fs');
const { timeStamp } = require('console');
const lander_leg = require('./lander_leg');
const ramped_pid_controller = require('./ramp_pid');
const { exit } = require('process');

class Spacecraft {

    // spacecraft configuration settings
    max_landing_velocity;
    starting_velocity;
    starting_altitude;
    starting_fuel;
    max_thrust;
    max_fuel_consumption;
    Beta1;
    Beta2;
    lander_mass;
    pid_Kp;
    pid_Ki;
    pid_Kd;
    pid_starting_alt;
    pid_final_alt;
    pid_starting_vel;
    pid_final_vel;

    // state variables
    #last_altitude = 0.0;
    current_velocity = 0.0;
    #last_velocity = 0.0;
    current_acceleration = 0.0;
    #current_drag_constant = 0.0;

    fuel_weight = 0.0;

    initial_touchdown = 0;
    parachute_deployed = 1;
    lander_legs_deployed = 0;
    backshell_ejected = 0;
    constant_velocity_phase = 0;
    #engine_cutoff_enable = 0;
    engine_cutoff = 1;
    engine_cutoff_state = 0;
    powered_descent = 0;
    #radar_lock = 0;
    #radar_altitude;
    #pid_controller;
    spacecraft_touchdown;
    leg1;
    leg2;
    leg3;
    #deploy_legs_commanded;
    landing_velocity;
    landing_time;
    planet;

    constructor(altitude, velocity, max_thrust, max_fuel, max_fuel_consumption, Beta1, Beta2, Kp, Ki, Kd, initial_alt, initial_vel, final_alt, final_vel, atmosphere_model) {

        this.leg1 = new lander_leg(8.0);
        this.leg2 = new lander_leg(10.0);
        this.leg3 = new lander_leg(18.0);

        this.starting_altitude = altitude;  // meters
        this.current_altitude = altitude;
        this.starting_velocity = velocity;  // meters/sec
        this.starting_mass = 420.9; // kg
        this.starting_thrust = 0.0; // 0.0 through 1.0
        this.planet = atmosphere_model;
        this.starting_fuel_weight = max_fuel // grams
        this.max_thrust = max_thrust;
        this.max_fuel_consumption = max_fuel_consumption; //131.0 * 12; // grams/sec
        this.thruster_setting = 0.0;
        this.Beta1 = Beta1;
        this.Beta2 = Beta2;
        this.pid_Kp = Kp;
        this.pid_Ki = Ki;
        this.pid_Kd = Kd;
        this.pid_final_vel = final_vel;
        this.pid_final_alt = final_alt;
        this.pid_starting_alt = initial_alt;
        this.pid_starting_vel = initial_vel;
    }

    reset() {

        this.current_altitude = this.starting_altitude;
        this.current_velocity = this.starting_velocity;
        this.lander_mass = this.starting_mass;
        this.fuel_weight = this.starting_fuel_weight;
        this.thrust = this.starting_thrust;
        this.#current_drag_constant = this.Beta1;
        this.lander_legs_deployed = 0;
        this.#deploy_legs_commanded = 0;
        this.#radar_lock = 0;
        this.spacecraft_touchdown = 0;
        this.constant_velocity_phase = 0;
        this.backshell_ejected = 0;
        this.#engine_cutoff_enable = 0;
        this.engine_cutoff_state = 0;
        this.engine_cutoff = 1;
        this.landing_time = 0;
        this.landing_velocity = 0;
        this.powered_descent = 0;
        this.total_time = 0.0;
        this.thruster_setting = 0.0;
        this.leg1.reset();
        this.leg1_last = 0;
        this.leg2.reset();
        this.leg2_last = 0;
        this.leg3.reset();
        this.leg3_last = 0;
        this.#pid_controller = undefined;
        this.initial_touchdown = 0;

    }


    dump_state() {

        console.log('altitude: ' + this.current_altitude);
        console.log('velocity: ' + this.current_velocity);
        console.log('fuel weight: ' + this.fuel_weight);
        console.log('lander_mass: ' + this.lander_mass);

    }

    calc_thrust(time_delta) {

        // no fuel, no thrust
        if (this.fuel_weight == 0.0) {

            return 0;
        }

        if (this.engine_cutoff) {

            return 0;
        }

        var fuel_needs = this.max_fuel_consumption * this.thruster_setting * time_delta;

        if (fuel_needs > this.fuel_weight) {

            fuel_needs = this.fuel_weight;
        }

        var thrust_force = this.max_thrust * this.thruster_setting;
        var thrust_acceleration = thrust_force / this.lander_mass;

        this.fuel_weight -= fuel_needs;

        // console.log(`Remaining fuel = ${this.fuel_weight}`)

        return thrust_acceleration;

    }

    deploy_legs() {

        // console.log("Time to deploy the legs");
        this.leg1.deploy();
        this.leg2.deploy();
        this.leg3.deploy();

    }

    update_altitude(time_delta) {

        // console.log(`altitude = ${this.current_altitude}, velocity = ${this.current_velocity}`);
        let atmospheric_drag = this.planet.atmosphere_drag( this.current_altitude, 
            this.current_velocity, 
            this.#current_drag_constant,
            this.lander_mass);

        let thruster_acceleration = this.calc_thrust(time_delta);

        // console.log(`acceleration = ${atmospheric_drag}`);

        let new_acceleration = this.planet.gravity_acceleration() + atmospheric_drag + thruster_acceleration;

        // console.log(`Total acceleration = ${new_acceleration}`);
        let new_velocity = this.current_velocity + (new_acceleration + this.current_acceleration) * time_delta * 0.5;
        let new_altitude = this.current_altitude + this.current_velocity * time_delta + 0.5 * new_acceleration * time_delta * time_delta;

        // console.log(`velocity = ${new_velocity} altitude = ${new_altitude}`);

        if ( Number.isNaN(new_altitude) || Number.isNaN(new_velocity)) {

            console.log("Bad calculation results...");
            return false;
            // exit(-1);
        }
        // console.log(`new_altitude = ${new_altitude}, new_velocity = ${new_velocity}`);

        // this is the instance it touches down so be sure to grab the landing velocity
        if ( this.initial_touchdown === 0 && this.current_altitude > 0.0 && new_altitude <= 0.0) {

            this.initial_touchdown = 1;
            this.landing_velocity = new_velocity;
            this.landing_time = this.current_time;

            // console.log("Landing");
            // console.log(this.landing_velocity);

            new_altitude = 0.0;

            // console.log(new_altitude);
            // console.log(this.current_altitude);
            // if the landing is too fast, its a crash
            if (Math.abs(new_velocity) > 2.6) {

                this.spacecraft_touchdown = 2;

            }
            else {

                console.log("Successful touchdown")
                this.spacecraft_touchdown = 1;
            }

            // if the legs weren't already deployed, its an error and a crash
            if (this.lander_legs_deployed != 1 ) {

                this.spacecraft_touchdown = 2;
                this.lander_legs_deployed = 2;
            }

        }

        if (new_altitude <= 0.0) {

            new_altitude = 0.0;
            new_velocity = 0.0;
            new_acceleration = 0.0;

        }

        if (new_altitude < 1425.0 & new_altitude > 30.0) {

            this.#radar_lock = 1;
            this.#radar_altitude = new_altitude;

        }
        else {

            this.#radar_lock = 0;
            this.#radar_altitude = Math.random() * 4000;
        }

        if (Math.abs(new_velocity) < Math.abs(this.pid_starting_vel) && Math.abs(new_velocity) > 2.40) {

            if (!this.#pid_controller) {

                this.#pid_controller = new ramped_pid_controller(this.pid_Kp, this.pid_Ki, this.pid_Kd, new_altitude, this.pid_final_alt, new_velocity, this.pid_final_vel);

                this.deploy_legs();
                this.backshell_ejected = 1;
                this.#current_drag_constant = this.Beta2;
                this.powered_descent = 1;
                this.engine_cutoff = 0;
            }

        }

        if (this.powered_descent) {

            var pid_output = this.#pid_controller.update(new_altitude, time_delta);

            // console.log(`pid_output = ${pid_output}`);
            this.thruster_setting = Math.min(pid_output, 1);

            if (this.thruster_setting < 0) {

                this.thruster_setting = 0.0;
            }        // if (new_altitude <= 0.0) {

                //     console.log("setting things to zero");
                //     new_altitude = 0.0;
                //     new_velocity = 0.0;
                //     new_acceleration = 0.0;
        
                // }

        }

        if (this.constant_velocity_phase == 1) {

            // console.log("CONSTANT VELOCITY");
            if (Math.abs(new_velocity) > 2.5) {

                this.thruster_setting = .8;

            }
            else {

                this.thruster_setting = .4;

            }
        }

        this.current_acceleration = new_acceleration;
        this.current_altitude = new_altitude;
        this.current_velocity = new_velocity;

        if (this.current_altitude <= 40.0 ) {

            this.#engine_cutoff_enable = 1;
        }

        if (this.current_velocity >= -2.5) {

            this.constant_velocity_phase = 1;

        }

        // console.log(`new altitude = ${this.current_altitude}, new velocity = ${this.current_velocity}, thruster = ${this.thruster_setting}`);
        return true;
    }

    update(time_delta) {

        this.total_time += time_delta;
        let result = this.update_altitude(time_delta);

        if (!result) {

            return false;
        }

        this.leg1.update(time_delta, this.current_altitude);
        this.leg2.update(time_delta, this.current_altitude);
        this.leg3.update(time_delta, this.current_altitude);

        if (this.leg3.lander_leg_deployed == 1 && this.leg2.lander_leg_deployed == 1 && this.leg1.lander_leg_deployed == 1) {

            this.lander_legs_deployed = 1;
        }

        if (this.leg1.leg_touchdown && this.leg1_last) {

            this.engine_cutoff = 1;
        }

        if (this.leg2.leg_touchdown && this.leg2_last) {

            this.engine_cutoff = 1;
        }

        if (this.leg3.leg_touchdown && this.leg3_last) {

            this.engine_cutoff = 1;
        }

        this.leg1_last = this.leg1.leg_touchdown;
        this.leg2_last = this.leg2.leg_touchdown;
        this.leg3_last = this.leg3.leg_touchdown;

        if (this.landing_time > 0 ) {

            if (this.current_time - this.landing_time > 0.40 && this.engine_cutoff == 0) {

                // console.log("Engine cuttof not performed in time")
                this.engine_cutoff_state = 2;
            }
        }

        return true;
    }

    get_current_stats() {


        var spacecraft_state = 0;

        if (this.#engine_cutoff_enable && this.engine_cutoff) {

            this.engine_cutoff_state = 1;
        }

        spacecraft_state |= (this.leg1.leg_touchdown & 1) << 15;
        spacecraft_state |= (this.leg2.leg_touchdown & 1) << 14;
        spacecraft_state |= (this.leg3.leg_touchdown & 1) << 13;
        spacecraft_state |= (this.spacecraft_touchdown & 3) << 11;
        spacecraft_state |= (this.parachute_deployed & 3 ) <<9;
        spacecraft_state |= (this.lander_legs_deployed & 3 ) << 7;
        spacecraft_state |= (this.backshell_ejected & 3 ) << 5;
        spacecraft_state |= (this.constant_velocity_phase & 3 ) << 3;
        spacecraft_state |= (this.engine_cutoff_state & 3 ) << 1;
        spacecraft_state |= (this.powered_descent & 1);

        var hexvalue = spacecraft_state.toString(16);

        // console.log(`spacecraft state = ${hexvalue}`);

        var stats = {
            'type': 'data',
            'altitude': this.current_altitude,
            'velocity': this.current_velocity,
            'fuel%': this.fuel_weight/this.starting_fuel_weight * 100,
            'thrust%': this.thruster_setting * 100,
            'spacecraft_state': hexvalue,
            'sim_time_delta': (this.total_time)
        };

        return stats;
 
    }

    get_controller_inputs() {

        let value =  `${this.#radar_lock},${this.#radar_altitude},${this.current_velocity},${this.leg1.leg_touchdown},${this.leg2.leg_touchdown},${this.leg3.leg_touchdown},${this.#engine_cutoff_enable}\n`

        return value;
    }
}

module.exports = Spacecraft
