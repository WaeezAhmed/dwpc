



const credentials=  [
  {
    app_user: "Production_User",
    app_password: "6515a828fcd7955efa8614e8fe32f6b6",
    role: "Production"
  },
  {
    app_user: "Support_User",
    app_password: "e556a8ac44843c4f0849a07e23c72948",
    role: "Support"
  },
  {
    app_user: "Demo_User",
    app_password: "fc9b98fd63286f67cfe9b212f31afe28",
    role: "Demo"
  }
]




document.getElementById("login-form").addEventListener("submit", loginForm)


  function loginForm(e){
e.preventDefault()
    let username = document.getElementById("username-login").value
    let password = document.getElementById("password-login").value

    const hashedPwd = CryptoJS.MD5(username + password).toString();

    // alert(hashedPwd)

      let targetObject = credentials.filter((elem) => {
        return elem.app_user === username;
      });
    
      if (
        username === targetObject[0].app_user &&
        hashedPwd === targetObject[0].app_password
      ) {
    
        localStorage.setItem("user", JSON.stringify(targetObject[0].role));

        if(targetObject[0].role === "Production"){
                      window.location.href= "admin.html"
          }
         else if(targetObject[0].role === "Support"){
                      window.location.href=  "service.html"
          }
          else{
                      window.location.href="dashboard.html"
         }
        }
        else{
          alert("Please provide correct login credentials")
        }




    }


    const showPassword = document.querySelector("#show-password")

    const passwordField = document.querySelector("#password-login")


    showPassword.addEventListener("click", function(){
      // this.classList.toggle("fa-eye-slash")

     const type=  passwordField.getAttribute("type") === "password" ? "text" : "password"

     passwordField.setAttribute("type", type)

     if(type == "password"){
      this.classList.remove("fa-eye")
      this.classList.add("fa-eye-slash")
     }
     else{
      this.classList.remove("fa-eye-slash")
      this.classList.add("fa-eye")
     }
    })





