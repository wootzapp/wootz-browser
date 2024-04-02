import {BrowserProxy} from './browser_proxy.js';

let numDonutsBaked: number = 0;

window.onload = function() {
  // Other page initialization steps go here
  const proxy = BrowserProxy.getInstance();
  // Tells the browser to start the pilot light.
  proxy.handler.startPilotLight();
  // Adds a listener for the asynchronous "donutsBaked" event.
  proxy.callbackRouter.donutsBaked.addListener(
    (numDonuts: number) => {
      numDonutsBaked += numDonuts;
    });
};

function CheckNumberOfDonuts() {
  // Requests the number of donuts from the browser, and alerts with the
  // response.
  BrowserProxy.getInstance().handler.getNumberOfDonuts().then(
      (numDonuts: number) => {
        alert('Yay, there are ' + numDonuts + ' delicious donuts left!');
      });
}

function BakeDonuts(numDonuts: number) {
  // Tells the browser to bake |numDonuts| donuts.
  BrowserProxy.getInstance().handler.bakeDonuts(numDonuts);
}
