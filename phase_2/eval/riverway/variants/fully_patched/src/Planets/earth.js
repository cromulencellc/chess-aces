
class Earth {


    Gsubm = -9.81;
    name = 'earth';

    atmosphere_density(altitude) {

        let temp;
        let pressure;

        if (altitude > 25000.0) {

            temp = -131.21  + .00299 * altitude;
            pressure = 2.488 * Math.pow(( temp + 273.1)/216.6, -11.388);
        }
        else if (altitude > 10000 ){

            temp = -56.46;
            pressure = 22.65 * Math.exp(1.73 - .000157 * altitude);
        }
        else {

            temp = 15.04 - .00649 * altitude;
            pressure = 101.29 * Math.pow( (temp + 273.1)/288.08, 5.256);
        }
        
        let density = pressure / ( .2869 * (temp+273.1));

        return density;
    }

    gravity_acceleration() {

        return this.Gsubm;

    }

    atmosphere_drag(altitude, velocity, drag_constant, mass) {


        let density = this.atmosphere_density(altitude);

        let drag_coefficient = density * drag_constant;

        let drag_force = 0.5 * drag_coefficient * velocity * velocity;

        let drag_accel = drag_force / mass;

        return drag_accel;
    }
}

module.exports = Earth