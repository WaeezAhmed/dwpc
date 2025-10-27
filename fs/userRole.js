

const activePage = window.location.pathname;


const navLinks = document.querySelectorAll("a").forEach(link => {

    if(link.href.includes(`${activePage}`)){
        link.classList.add("active")
    }
})


userRole = JSON.parse(localStorage.getItem("user") || null)



if(userRole ==="Support"){
 document.getElementById("inference-nav").style.display= "none"
 document.getElementById("inference-nav-2").style.display= "none"
 
 document.getElementById("admin-nav").style.display= "none"

}

if(userRole ==="Demo"){
    document.getElementById("inference-nav").style.visibility= "hidden"
    document.getElementById("inference-nav-2").style.visibility= "hidden"
    document.getElementById("admin-nav").style.visibility= "hidden"
    document.getElementById("service-nav").style.visibility= "hidden"
    document.getElementById("mqtt-1-nav").style.visibility= "hidden"
    document.getElementById("mqtt-2-nav").style.visibility= "hidden"
    document.getElementById("wifi-nav").style.visibility= "hidden"
   
   }



 let choosenVariant =   JSON.parse(localStorage.getItem("variant") || null)


 if(choosenVariant === "ZIGBEE"){
    document.getElementById("mqtt-1-nav").style.display= "none"
    document.getElementById("mqtt-2-nav").style.display= "none" 
    document.getElementById("wifi-nav").style.display= "none"
   
   }
