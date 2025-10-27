


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

    let commandOutput1  = JSON.parse(message.data);

    let commandOutput = message.data;

   
    


     // Clear the terminal
     var terminal = document.getElementById('terminal');
    //  terminal.textContent = '';
   
     // Simulating command execution and output
   //   var output = 'Output for ' + command + ' command';
   
     // Create a new terminal output element
     var terminalOutput = document.createElement('div');
     terminalOutput.className = 'terminal-output';
    //  terminalOutput.textContent = JSON.stringify(output, null, 2);
    terminalOutput.textContent =  commandOutput;
   
   
     // Append the output to the terminal
     terminal.appendChild(terminalOutput);
   
      // Scroll to the bottom of the terminal
   terminal.scrollTop = terminal.scrollHeight;
  };

  // To raise an alert pop-up window when the web sockkets connection is closed
  ws.onerror = (message) => {
    alert("Unexpectedly websockets is closed, Please refresh the page..!!");
  };









function executeCommandGetInfo(){


    fetch("/rpc/sysGetInfo")
  
    .then((res)=> {
    if(res.status === 200){
        return res.json()
    }else if(res.status === 401){
        window.location.href="./login.html"
      }
    else{
        alert("something went wrong")  ;
    }})
    .then((data)=>{
      alert("system Getinfo sucess");

    })
    .catch(err => console.log(err))

}

function executeCommandSetDebug(){


  fetch("/rpc/debug")

  .then((res)=> {
  if(res.status === 200){
      return res.json()
  }else if(res.status === 401){
      window.location.href="./login.html"
    }
  else{
      alert("something went wrong")  ;
  }})
  .then((data)=>{
    alert("Debug done sucessfully");

  })
  .catch(err => console.log(err))

}