
class Ramped_PID {

        Kp;
        Ki;
        Kd;
        #delta_altitude;
        #delta_velocity;
        #planned_acceleration;
        initial_altitude;
        initial_velocity;
        final_altitude;
        final_velocity;
        #last_time;
        #last_error;
        #error_integral;
        #position_error;
        #current_time = 0;

    constructor(Kp, Ki, Kd, initial_altitude, final_altitude, initial_velocity, final_velocity) {

        this.Kp = Kp;
        this.Ki = Ki;
        this.Kd = Kd;
        this.initial_altitude = initial_altitude;
        this.final_altitude = final_altitude;
        this.initial_velocity = initial_velocity;
        this.final_velocity = final_velocity;

        this.reset();

        // console.log(` planned_accel = ${this.#planned_acceleration} delta_velocity = ${this.#delta_velocity}, delta_altitude = ${this.#delta_altitude}`);

    }

    reset() {

        this.#delta_altitude = this.final_altitude - this.initial_altitude;
        this.#delta_velocity = this.final_velocity - this.initial_velocity;
        this.#planned_acceleration = (this.final_velocity * this.final_velocity- this.initial_velocity * this.initial_velocity)/ 2 / this.#delta_altitude;

        this.#current_time = 0;
        this.#last_error = 0;
        this.#error_integral = 0;
        this.#position_error = 0;

    }


    planned_altitude() {

        var planned_position = this.initial_altitude + this.initial_velocity * this.#current_time + .5 * this.#planned_acceleration * this.#current_time * this.#current_time;
        return planned_position;

    }

    update(current_altitude, delta_time) {


        this.#current_time += delta_time;

        let position_error = this.planned_altitude() - current_altitude;

        // console.log(`position error = ${position_error}`);

        this.#error_integral += position_error * delta_time;

        // console.log(`error integral= ${this.#error_integral}`);

        let error_derivative = (Math.abs(position_error) - Math.abs(this.#last_error))/delta_time;
        // console.log(`last_error = ${this.#last_error}, this error = ${position_error}`);

        // console.log(`error derivative= ${error_derivative}`);

        this.#last_error = position_error;

        var output = position_error * this.Kp + this.#error_integral * this.Ki + error_derivative * this.Kd;

        // console.log(output);

        return output;

    }

}

module.exports = Ramped_PID
