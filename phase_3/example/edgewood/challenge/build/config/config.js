global.db_url = process.env.MONGO_URL || "mongodb://mongodb:27017/weatherdb";
global.weather_port = parseInt(process.env.WEATHER_PORT || '50222');
global.express_port = parseInt(process.env.HTTP_PORT || '8080');
