// глобальная переменная для заполнения json-реквеста для 1 страницы
var first_page_json = JSON.parse(
  '{"contractType":"RUSTONN_PHYS_PERSON", "currencyType":"", "currencyKind":"", "contractDuration":2}'
);
var first_page_json_response;

// проверяем контракт при инициализации приложения
if (first_page_json.contractType != "PUGACHEV_PHYS_PERSON") {
  // если выбран контракт, отличный от "PUGACHEV_PHYS_PERSON", отключаем выбор валюты USDT
  document.getElementById("USDT").disabled = true;
  document.getElementById("USDT").classList.add("button-currency-disabled");
}

function setToDefault()
{
  first_page_json.contractType = "RUSTONN_PHYS_PERSON";
  first_page_json.currencyType = "";
  first_page_json.currencyKind = "";
  first_page_json.contractDuration = 2;

  document.getElementById("USDT").disabled = true;
  document.getElementById("USDT").classList.add("button-currency-disabled");
  // снимаем активность с кнопок
  document.getElementById("USDT").classList.remove("button-currency-active");
  document.getElementById("ROUBLES").classList.remove("button-currency-active");
  document.getElementById("DOLLARS").classList.remove("button-currency-active");
  document.getElementById("EURO").classList.remove("button-currency-active");

  // сбрасываем состояние кнопок и выбранного типа оплаты
  document.getElementById("CASH").classList.remove("button-money-disabled");
  document.getElementById("NON_CASH").classList.remove("button-money-active");
  document.getElementById("CASH").classList.remove("button-money-active");
  const hint = document.getElementById("city-hint");
  hint.style.visibility = "hidden";


  document.getElementById("CASH").disabled = false;
}

// функция для отображения хинта для типа контракта
function contractHint() {
  const selectElement = document.getElementById("document_type_select");
  selectElement.addEventListener("change", (event) => {
    setToDefault();
    const hint = document.getElementById("contract-hint");
    if (event.target.value == "RUSTONN_PHYS_PERSON")
    {
      hint.style.visibility = "visible";
    }
    else
    {
      hint.style.visibility = "hidden";
      if (event.target.value == "PUGACHEV_PHYS_PERSON")
      {
        // если выбран контракт "PUGACHEV_PHYS_PERSON", разрешаем выбирать валюту USDT
        document.getElementById("USDT").disabled = false;
        document.getElementById("USDT").classList.remove("button-currency-disabled");
        //document.getElementById("USDT").classList.remove("button-currency-active");
      }
    }

    // устанавливаем тип контракта
    first_page_json.contractType = event.target.value;
    console.log(first_page_json.contractType);
  });
}

// функция обработки валюты
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
    clear();
    event.target.classList.toggle("button-currency-active");
    // заполняем поле в JSON
    first_page_json.currencyType = event.target.id;
    console.log(first_page_json.currencyType);
 
    if (event.target.id != "USDT")
    {
      // сбрасываем состояние кнопок и выбранного типа оплаты
      document.getElementById("CASH").classList.remove("button-money-disabled");
      document.getElementById("NON_CASH").classList.remove("button-money-active");
      document.getElementById("CASH").disabled = false;
    }
    // если выбрана валюта USDT, отключаем кнопку перевода займа
    else {
      document.getElementById("CASH").classList.add("button-money-disabled");
      document.getElementById("CASH").disabled = true;
      // автоматически выбираем наличную оплату
      // спровоцируем клик-событие внутри функции cash()
      document.getElementById("NON_CASH").click();
    }
  });
  }
 }

// функция обработки вкладки выберите способ оплаты
function money() {
  cash();
  nonCash();
}

// функция обработки срока договора
function contractDur() {
  duration = document.getElementById("contract_term");
  duration.addEventListener("change", (event) => {
      first_page_json.contractDuration = event.target.value;
      console.log(first_page_json.contractDuration);
  });
}

// наличный рассчет
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

// безналичный рассчет
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

// проверяем, что все поля были заполнены
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
      first_page_json.currencyKind +
      " " +
      first_page_json.contractDuration
  );
  if (isAllFieldsFilled(first_page_json))
  {
    console.log(JSON.stringify(first_page_json));
    // отправить хттп запрос с json-файлом

    fetch("/api/v1/prog/tag_values", {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        Accept: "application/json",
      },
      body: JSON.stringify(first_page_json)
    })
    .then((response) => response.json())
    .then((data) => {
      first_page_json_response = data;
      localStorage.setItem("first_page_json_response", JSON.stringify(first_page_json_response));
      console.log(first_page_json_response); // Вывод данных для проверки
    })
    .catch((error) => {console.error("Error:", error);
    return;});
    // переходим на некст страницу
    window.location.href = "second_page.html";
  }
}

contractHint();
contractDur();
currency();
money();
