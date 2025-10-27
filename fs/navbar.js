

function Navbar() {

  return `<div class="container md-navbar d-flex justify-content-evenly">
  
   
      <div class="navbar-brand ms-0" >
  
        Flamencotech
      </div>
  
  
        <div class="d-flex">
         
    
          <a class="btn btn-primary px-3 me-2" id="admin-nav" href="./admin.html" role="button" > 
         
          
          Admin </a>
     
          <a class="btn btn-primary px-3 me-2 service-nav" id="service-nav" href="./service.html" role="button" > 
          

          Services </a>


          <a class="btn btn-primary px-3 me-2"  id="inference-nav" href="./inference.html" role="button">  Inference </a>

          <a class="btn btn-primary px-3 me-2"  id="inference-nav-2" href="./inference2.html" role="button">  Inference-2 </a>
          <a class="btn btn-primary px-3 me-2 mqtt-nav" id="mqtt-1-nav"  href="./mqtt.html" role="button"> MQTT-1</a>

            
            <a class="btn btn-primary px-3 me-2 mqtt-nav" id="mqtt-2-nav" href="./mqtttwo.html" role="button">  MQTT-2
            
            </a>


          <a class="btn btn-primary px-3 me-2 network-nav" id="wifi-nav"  href="./network.html" role="button"> Network</a>
          <a class="btn btn-primary px-3 me-2" id="dashboard-nav" href="./dashboard.html" role="button"> Dashboard</a>
          <a class="btn btn-primary px-3 me-2" id="systemhealth-nav" href="./systemhealth.html" role="button"> System-health</a>
          <a class="btn btn-primary px-3 me-2" id="logout-nav" role="button"> Logout</a>
    


   
  
         
        </div>
  
    </div>`


}


// const nav= document.querySelector(".navbar")
// fetch("./navbar.html")
// .then(res=> res.text())
// .then(data=> {
//     nav.innerHTML = data;
// })




export {Navbar};

