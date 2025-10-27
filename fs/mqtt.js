
let userRole = JSON.parse(localStorage.getItem("user") || null)


if(userRole ==="Demo"){
 document.getElementById("mqtt-1-page").style.display= "none"
 
}



function getMqttData(){

    
        fetch("/rpc/Config.Get", {
            method: 'GET'
        })
        .then((response)=> {
  
          if(response.status === 200){
              return response.json()
          }
          else if(response.status === 401){
            window.location.href="./login.html"
          }
          else{
              alert("something went wrong")  ;
          }})
            .then(response => {

            document.getElementById("mqtt-broker").value = response.mqtt.server
            // document.getElementById("mqtt-port").value = response.mqtt.port

            document.getElementById("mqtt-status").value  = response.mqtt.status
            document.getElementById("mqtt-people-count").value = response.mqtt.pub
            document.getElementById("response-1").value = response.mqtt.sub
            // document.getElementById("mqtt-protocol").value = response.mqtt.protocol
            document.getElementById("mqtt-device-health").value = response.mqtt.device_health
           document.getElementById("mqtt-user-name").value = response.mqtt.user
            document.getElementById("mqtt-user-password").value = response.mqtt.pass
            // document.getElementById("mqtt-cert").value = response.mqtt.ca
            document.getElementById("qos-level").value = response.mqtt.max_qos
            document.getElementById("client-id").value = response.mqtt.client_id

            


       

        })
            .catch(err => console.error(err));
      }
    
    
      getMqttData()



     


document.getElementById("mqtt-form").addEventListener("submit",  setMqttBrokerOneForm)


function  setMqttBrokerOneForm(e){
e.preventDefault()
  let mqttBroker = document.getElementById("mqtt-broker").value
  let mqttPeopleCount= document.getElementById("mqtt-people-count").value
  let mqttDeviceHealth= document.getElementById("mqtt-device-health").value
  let mqttUserName= document.getElementById("mqtt-user-name").value
  let mqttUserPassword = document.getElementById("mqtt-user-password").value
  let mqttResponse = document.getElementById("response-1").value
  let qosResponse = document.getElementById("qos-level").value
  let clientId = document.getElementById("client-id").value


var mqttBrokerOneData = JSON.stringify({
  config:{
    mqtt: {
      "server": mqttBroker,
      "pub": mqttPeopleCount,
      "device_health": mqttDeviceHealth,
      "user": mqttUserName,
      "pass": mqttUserPassword,
      "sub" : mqttResponse,
      "max_qos": parseInt(qosResponse),
      "client_id": clientId
    }
  }
 
  });

  // let BearerCheck = JSON.parse(localStorage.getItem("token") || null)
    fetch("/rpc/Config.Set", {
        method: "POST",
        headers: {
            "Accept": "application/json, text/plain, */*",
            "Content-type": "application/json"

        },
        body: mqttBrokerOneData
    })
    .then((res)=> {
    if(res.status === 200){
        return res.json()
    }
    else if(res.status === 401){
      window.location.href="./login.html"
    }
    else{
        alert("something went wrong")  ;
    }})
    .then((data)=>{

      
      let new_data = JSON.stringify({ reboot: false })
      fetch("/rpc/Config.Save", {
        method: "POST",
        headers: {
            "Accept": "application/json, text/plain, */*",
            "Content-type": "application/json"
        },
        body: new_data 

    })
    .then((res)=> {
      if(res.status === 200){
      return res.json()
      }
      else if(res.status === 401){
        window.location.href="./login.html"
      }
      else{
          alert("something went wrong")  ;
      }})
      .then((data)=>{
 
        alert("MQTT  parameter is set sucessfully");
      })
      .catch(err => console.log(err))
    })
    .catch(err => console.log(err))


    }








document.getElementById("upload-cert-button").addEventListener("click",  setMqttProtocolCert)


async function  setMqttProtocolCert(){


  const file =  mqtt_cert_file.files[0]

  const base64Data = await encodeAsBase64(file);

  if (base64Data.length < 10240) {
    if (base64Data.length > 2048) {
      postLargeData(file.name, base64Data, 2048);
    } else {
      fetch("/rpc/FS.Put", {
        method: "POST",
        headers: {
            "Accept": "application/json, text/plain, */*",
            "Content-type": "application/json"
    
        },
        body: JSON.stringify({
          filename: file.name,
          append: true,
          data: base64Data,
        })
    })
    .then((res)=> {
    if(res.status === 200){
        return res.json()
    }
    else if(res.status === 401){
      window.location.href="./login.html"
    }
    else{
        alert("something went wrong")  ;
    }})
    .then((data)=>{
      alert("MQTT certificate uploaded successfully");

      
     
    })
    .catch(err => console.log(err))

    }
  } else {
    alert("File size exceeds the limit, Please upload file lessthan 10KB ");
  }





    }



    /*********************************************************************
 * @fn      encodeAsBase64
 *
 * @brief   Encodes the user uploaded file date into base64 format
 *
 * @param   file -> user uploaded file
 *
 * @return  None.
 */
const encodeAsBase64 = (file) =>
new Promise((resolve, reject) => {
  const reader = new FileReader();
  reader.readAsDataURL(file);
  reader.onload = () =>
    resolve(reader.result.replace(/^data:.+;base64,/, ""));
  reader.onerror = (error) => reject(error);
});


/*********************************************************************
 * @fn      postLargeData
 *
 * @brief   To procees the large files into chuncks
 *
 * @param   fileName -> get the user uploaded file by its name
 *
 * @param   encodedData -> get the user uploaded file data in base64 encoded data format
 *
 * @param   chunkSize -> get the encoded data as chucks
 *
 * @return  None.
 */



const postLargeData = (fileName, encodedData, chunkSize) => {

  fetch("/rpc/FS.Put", {
    method: "POST",
    headers: {
        "Accept": "application/json, text/plain, */*",
        "Content-type": "application/json"

    },
    body: JSON.stringify({
      filename: fileName,
      append: true,
      data: encodedData.slice(0, chunkSize),
    })
})
.then((res)=> {
if(res.status === 200){
    return res.json()
}
else if(res.status === 401){
  window.location.href="./login.html"
}
else{
    alert("something went wrong")  ;
}})
.then((data)=>{


  if (
    chunkSize < encodedData.length &&
    encodedData.length - chunkSize > chunkSize
  ) {
    postLargeData(fileName, encodedData.slice(chunkSize), chunkSize);
  }
  else{



  fetch("/rpc/FS.Put", {
    method: "POST",
    headers: {
        "Accept": "application/json, text/plain, */*",
        "Content-type": "application/json"

    },
    body: JSON.stringify({
      filename: fileName,
      append: true,
      data: encodedData.slice(chunkSize),
    })
})
.then((res)=> {
if(res.status === 200){
    return res.json()
}
else if(res.status === 401){
  window.location.href="./login.html"
}
else{
    alert("something went wrong")  ;
}})
.then((data)=>{
  console.log(encodedData.slice(chunkSize));
  
 
})
.catch(err => console.log(err))
}
})
.catch(err => console.log(err))

};













    