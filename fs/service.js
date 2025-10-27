
let userRole = JSON.parse(localStorage.getItem("user") || null)



if(userRole ==="Demo"){
 document.getElementById("service-page").style.display= "none"
}
function getServiceData(){

      fetch("/rpc/Config.Get", {
      method: 'GET',
      // headers: {
      //   Authorization: `Bearer ${BearerCheck}`,
      // },
    })
    .then((response)=> {
    
      if(response.status === 200){
          return response.json()
      }
      else if(response.status === 401){
        window.location.href="./login.html"
      }
      else{
          alert("something went wrong");
      }})
  
      .then(response => {

        
        document.getElementById("threshold-caliberate-status").value = response.service.thresholdCalibration_status
        document.getElementById("crosstalk-caliberate-status").value = response.service.xtalkcalibration_status
        document.getElementById("reset-status").value  =  response.service.reset_status
        document.getElementById("livestream-toggle-status").value  =  response.service.livestream_toggle_status

      })
      .catch(err => console.error(err));
    }
  
  
    getServiceData()
  


document.getElementById("threshold-caliberate-button").addEventListener("click", getThresholdCalliberateValue)


function getThresholdCalliberateValue(){



    fetch("/rpc/thresholdCalibration", {
        method: "GET"
    })
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

      alert("threshold calibration is done sucessfully");
    })
    .catch(err => console.log(err))

}


document.getElementById("crosstalk-caliberate-button").addEventListener("click", getCrosstalkCalliberateValue)


function getCrosstalkCalliberateValue(){



    fetch("/rpc/xtalkCalibration", {
        method: "GET"
    })
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
    
      alert("crosstalk calibration is done sucessfully");
    })
    .catch(err => console.log(err))

}



document.getElementById("reset-button").addEventListener("click", getResetValue)


function getResetValue(){

   


    fetch("/rpc/factoryReset")
   
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
      alert("Factory Reset is done sucessfully");


    })
    .catch(err => console.log(err))

}



document.getElementById("reboot-button").addEventListener("click", getRebootValue)


function getRebootValue(){

 


    fetch("/rpc/rebootSensor", {
        method: "GET"
    
    })
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
      alert("Reboot is done sucessfully");

    })
    .catch(err => console.log(err))

}



document.getElementById("livestream-toggle-button").addEventListener("click", liveStreamToggleStatus)


function liveStreamToggleStatus(){

    fetch("/rpc/livestream", {
        method: "GET"
      
    })
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
      alert("livestream togged sucessfully");
  
    })
    .catch(err => console.log(err))

}