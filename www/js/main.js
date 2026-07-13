const button = document.getElementById("testButton");
const message = document.getElementById("message");

if (button && message) {
    button.addEventListener("click", () => {
        message.textContent = "JavaScript 文件加载成功";
    });
}
