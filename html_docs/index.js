// 写一个javascript函数，插入一个超链接指向网址https://github.com/doctordragon666
function insertLink() {
    var link = document.createElement("a");
    link.href = "https://github.com/doctordragon666";
    link.textContent = "GitHub";
    document.body.appendChild(link);
}