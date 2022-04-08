
class Mars {


    Gsubm = -3.7245;
    // Gsubm = undefined;
    name = 'mars';
    
    atmosphere_density(altitude) {

        let temp;

        if (altitude > 7000.0) {

            temp = -23.4 - .00222 * altitude;
        }
        else {

            temp = -31.0 - .000998 * altitude;
        }

        let pressure = .699 * Math.exp( -.00009 * altitude);
        let density = pressure / ( .1921 * (temp+273.1))

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

module.exports = Mars