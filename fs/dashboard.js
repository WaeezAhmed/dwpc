


//  Open a WebSocket to the server.
  //  Need a Regular Expression to get the URL to open the WebSocket to
  //  Matches 192.168.1.8 etc.
  const serverUrlRegex = /\d+\.\d+\.\d+\.\d+/;
  //  Get the URL from the browser.
  const currentUrl = window.location.origin;
  console.log(`The currentURL is ${currentUrl}.`);
  console.log(typeof(currentUrl))
  //  Extract what is needed to create WebSocket.
  const serverUrl = currentUrl.match(serverUrlRegex);
  //  The match is in the 0th element of the array.
  console.log(`The server URL is ${serverUrl[0]}`);
  console.log(`The server URL is ${serverUrl} serverURL`);
  //  Comment this for static URI.
  let ws = new WebSocket(`ws://${serverUrl}`);
  ws.onopen = () => {
    console.log("Web browser opened a WebSocket.");
  };

  ws.onclose = () => {
    console.log("Web browser WebSocket just closed.");
    alert("Unexpectedly websockets has closed, Please referesh the page..!!");
  };
  // Display the data from the server in web page
  ws.onmessage = (message) => {
    console.log(message.data);
    let dwpc = JSON.parse(message.data);
    
    console.log("DWPC Data: ", dwpc);

    if(dwpc.grid && dwpc.grid.length > 1){
      let  one_d_array = dwpc.grid
   
      if(dwpc.grid.length ===64){
        const container = document.querySelector(".grid-container");
        container.innerHTML= ""
        container.classList.remove("display_none");
        for (let i = 0; i < 64; i++) {
          const item = document.createElement("div");
          item.classList.add("grid-item");
        
        let color = Math.floor((one_d_array[i]/ 10)+ 60)
        
        console.log(one_d_array, "one_d_array")
        console.log(one_d_array[i], color, "color")
        
        
          item.style.backgroundColor= `hsl(${color}, 50%, 50%)`;
          
          // item.innerHTML = one_d_array[i] + ` (${i})`
          item.innerHTML = one_d_array[i]
          container.appendChild(item);
        }
      }
      else{
        const container = document.querySelector(".grid-container1");
        container.innerHTML= ""
        container.classList.remove("display_none");
           for (let i = 0; i < 16; i++) {
             const item = document.createElement("div");
             item.classList.add("grid-item");
           
           let color = Math.floor((one_d_array[i]/ 10)+ 60)
             
             item.style.backgroundColor= `hsl(${color}, 50%, 50%)`;
             
             item.innerHTML = one_d_array[i]
             container.appendChild(item);
           }
      }
    }
    else{


      let dwpc_date = new Date(dwpc.time);
  
      // if(dwpc.absloute && dwpc.incount && dwpc.incount && dwpc.time ){
        console.log(dwpc.absloute, "dwpc.absloute", dwpc.incount, "dwpc.incount", dwpc.outcount, "dwpc.outcount", dwpc_date, "dwpc_date")
        document.getElementById("footfall-today").innerHTML =  dwpc.absloute
        document.getElementById("dashboard-entry").innerHTML = dwpc.incount
        document.getElementById("dashboard-exit").innerHTML = dwpc.outcount
        document.getElementById("time-stamp").innerHTML =   
         dwpc_date.getHours() +
        ":" +
        dwpc_date.getMinutes() +
        ":" +
        dwpc_date.getSeconds();
      }
   


    // }
 





  
  };

  // To raise an alert pop-up window when the web sockkets connection is closed
  ws.onerror = (message) => {
    alert("Unexpectedly websockets is closed, Please refresh the page..!!");
  };


