

exports.getTimestamp = function() {

    let now = new Date();
  
    const year = now.getUTCFullYear();
    const day = now.getUTCDate().toString().padStart(2, "0");
    const month = (now.getUTCMonth() + 1).toString().padStart(2, "0");
    const hour = now.getUTCHours().toString().padStart(2, "0");
    const mins = now.getUTCMinutes().toString().padStart(2, "0");
    const seconds = now.getUTCSeconds().toString().padStart(2, "0");
    const milliseconds = now.getUTCMilliseconds().toString().padStart(3, "0");

    return `${year}${month}${day}-${hour}${mins}${seconds}.${milliseconds}`
  }

