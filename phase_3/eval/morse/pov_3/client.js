const puppeteer = require('puppeteer');

setTimeout(() => {  console.log("Client Started"); 

(async () => {
  const browser = await puppeteer.launch({
    args: [
      // Required for Docker version of Puppeteer
      '--no-sandbox',
      '--disable-setuid-sandbox',
      // This will write shared memory files into /tmp instead of /dev/shm,
      // because Docker’s default for /dev/shm is 64MB
      '--disable-dev-shm-usage'
    ]
  });
  const page = await browser.newPage();
  page.on('dialog', async dialog => {
    console.log(dialog.message());
    await dialog.dismiss();
  });
  //await page.goto('http://172.21.0.3:8888?c=asdfasdf'); // wait until page load
  await page.goto('http://challenge_container:4567/login', { waitUntil: 'networkidle0' }); // wait until page load
  await page.type('#username', 'altchess');
  await page.type('#password', 'altchess123');
  await page.click('#login');

  await page.waitForNavigation(); // <------------------------- Wait for Navigation
  console.log('New Page URL:', page.url());
  await page.goto('http://challenge_container:4567/user/chess/');
  const pageContents = await page.content();
  //await page.reload({ waitUntil: ["networkidle0", "domcontentloaded"] });
  //const pageContents1 = await page.content();
  await browser.close();
  
  //console.log(pageContents);
})();

}, 3000);