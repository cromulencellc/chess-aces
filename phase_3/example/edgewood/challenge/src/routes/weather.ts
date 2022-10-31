
const express = require('express');

import weatherController from '../controllers/weather'

let router = express.Router();

router.get('/weather/temperature', weatherController.get_current_temperature);
router.get('/weather/max_temperature', weatherController.get_max_temperature);
router.get('/weather/temperature/start/:start/end/:end', weatherController.get_temperature_from);
router.get('/weather/wind', weatherController.get_current_wind);
router.get('/weather/max_wind', weatherController.get_max_wind);
router.get('/weather', weatherController.get_summary_data);

module.exports = router;