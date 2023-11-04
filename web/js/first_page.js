// глобальная переменная для заполнения json-реквеста для 1 страницы
var first_page_json = JSON.parse(
  '{"contractType":"RUSTONN_PHYS_PERSON", "currencyType":"", "currencyKind":"", "contractDuration":1}'
);
var first_page_json_response;

// функция для отображения хинта для типа контракта
function contract() {
  const selectElement = document.getElementById("document_type_select");
  selectElement.addEventListener("change", (event) => {
    const hint = document.getElementById("contract-hint");
    first_page_json.contractType = event.target.value;
    // устанавливаем тип контракта
    console.log(first_page_json.contractType);
  });
}

// функция для обработки валюты
function currency() {
  function clear() {
    buttons = document.querySelectorAll(".button-currency");
    buttonsArray = Array.from(buttons);
    buttonsArray.forEach((button, index) => {
      button.classList.remove("button-currency-active");
    });
  }
  const curCurrency = document.getElementsByClassName("button-currency");
  for (var i = 0; i < curCurrency.length; ++i) {
    curCurrency[i].addEventListener("click", (event) => {
      // Check if the clicked target is a button
      //console.log(`Clicked button: ${event.target.id}`);
      clear();
      event.target.classList.toggle("button-currency-active");
      // заполняем поле в JSON
      first_page_json.currencyType = event.target.id;
      console.log(first_page_json.currencyType);
    });
  }
}

function money() {
  cash();
  nonCash();
}

function cash() {
  button = document.getElementById("CASH");
  console.log("cash");
  button.addEventListener("click", function () {
    const hint = document.getElementById("city-hint");
    hint.style.visibility = "visible";
    button.classList.toggle("button-money-active");

    NON_CASH = document.getElementById("NON_CASH");
    NON_CASH.classList.remove("button-money-active");
    // заполняем json
    first_page_json.currencyKind = "CASH";
    console.log(first_page_json.currencyKind);
  });
}

function nonCash() {
  const button = document.getElementById("NON_CASH");
  console.log("nonCash");
  button.addEventListener("click", function () {
    const hint = document.getElementById("city-hint");
    hint.style.visibility = "hidden";
    button.classList.toggle("button-money-active");

    CASH = document.getElementById("CASH");
    CASH.classList.remove("button-money-active");
    // заполняем json
    first_page_json.currencyKind = "NON_CASH";
    console.log(first_page_json.currencyKind);
  });
}

function isAllFieldsFilled(jsonObj) {
  for (let key in jsonObj) {
    if (
      jsonObj[key] === "" ||
      jsonObj[key] === null ||
      jsonObj[key] === undefined
    ) {
      return false;
    }
  }
  return true;
}

// функция перехода на следующую страницу
//@todo добавить тут отсылку хттп запроса
function onNextPage() {
  console.log(
    first_page_json.contractType +
      " " +
      first_page_json.currencyType +
      " " +
      first_page_json.currencyKind
  );
  if (isAllFieldsFilled(first_page_json))
  {
    console.log(JSON.stringify(first_page_json));
    // отправить хттп запрос с json-файлом

    $.post({
      url: '/api/v1/prog/tag_values',
      dataType: 'json',
      contentType:"application/json",
      data: JSON.stringify(first_page_json)
    }).done(function(data){
      first_page_json_response = data;
      localStorage.setItem("first_page_json_response", JSON.stringify(first_page_json_response));
      console.log(first_page_json_response); // Вывод данных для проверки
      // переходим на некст страницу
      window.location.href = "second_page.html";
    })
    .fail(function(){
        alert("Can't load contract");
    });

  }
}

contract();
currency();
money();
