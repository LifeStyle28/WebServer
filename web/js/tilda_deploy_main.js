// глобальная переменная для заполнения json-реквеста для 1 страницы
var first_page_json = JSON.parse(
  '{"contractType":"", "currencyType":"", "currencyKind":"", "contractDuration":2}'
);

// переменная для ограничение вариантов выбора
var currentContracts = [
  {
    contractType: "RUSTONN_PHYS_PERSON",
    currencyType: ["ROUBLES"],
    currencyKind: ["CASH", "NON_CASH"],
  },
  {
    contractType: "RUSTONN_LAW_PERSON",
    currencyType: ["ROUBLES"],
    currencyKind: ["NON_CASH"],
  },
  {
    contractType: "PUGACHEV_PHYS_PERSON",
    currencyType: ["ROUBLES", "DOLLARS", "EURO", "USDT"],
    currencyKind: ["CASH", "NON_CASH"],
  },
];

var usdtLogic = [
  {
    contractType: "PUGACHEV_PHYS_PERSON",
    currencyType: ["ROUBLES", "DOLLARS", "EURO", "USDT"],
    currencyKind: ["NON_CASH"],
  },
];

function setToDefault() {
  first_page_json.contractType = "";
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

function disableButtons(result) {
  if (result !== undefined) {
    console.log(
      "Array " +
        Array.from(document.querySelectorAll(".button-currency")).map(
          (button) => button.id
        )
    );
    console.log("curArray " + result.currencyType);
    Array.from(document.querySelectorAll(".button-currency"))
      .map((button) => button.id)
      .forEach((id) => {
        // id - все элементы
        //console.log("id " + id);
        if (result.currencyType.find((kind) => kind === id) !== undefined) {
          console.log("id2 " + id);
          document
            .getElementById(id)
            .classList.remove("button-currency-disabled");
          document.getElementById(id).disabled = false;
        } else {
          document.getElementById(id).classList.add("button-currency-disabled");
          document.getElementById(id).disabled = true;
        }
      });

    Array.from(document.querySelectorAll(".button-money"))
      .map((button) => button.id)
      .forEach((id) => {
        // id - все элементы
        if (result.currencyKind.find((kind) => kind === id) !== undefined) {
          console.log("id3 " + id);
          document.getElementById(id).classList.remove("button-money-disabled");
          document.getElementById(id).disabled = false;
        } else {
          document.getElementById(id).classList.add("button-money-disabled");
          document.getElementById(id).disabled = true;
        }
      });
  }
}

// функция для отображения хинта для типа контракта
function contractHint() {
  function clear() {
    buttons = document.querySelectorAll(".button-docs-selector");
    buttonsArray = Array.from(buttons);
    buttonsArray.forEach((button, index) => {
      button.classList.remove("button-docs-selector-active");
    });
  }
  const curCurrency = document.getElementsByClassName("button-docs-selector");
  for (var i = 0; i < curCurrency.length; ++i) {
    curCurrency[i].addEventListener("click", (event) => {
      clear();
      event.target.classList.toggle("button-docs-selector-active");
      // заполняем поле в JSON
      first_page_json.contractType = event.target.id;
      console.log("id1 " + first_page_json.contractType);

      // если выбрана валюта USDT, отключаем кнопку перевода займа
      if (event.target.id == "USDT") {
        const usdtObject = usdtLogic.find(
          (obj) => obj.contractType === first_page_json.contractType
        );
        disableButtons(usdtObject);
        const hint = document.getElementById("city-hint");
        hint.style.visibility = "hidden";
      } else {
        const nonUsdtObject = currentContracts.find(
          (obj) => obj.contractType === first_page_json.contractType
        );
        disableButtons(nonUsdtObject);
      }
    });
  }
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

      // если выбрана валюта USDT, отключаем кнопку перевода займа
      if (event.target.id == "USDT") {
        const usdtObject = usdtLogic.find(
          (obj) => obj.contractType === first_page_json.contractType
        );
        disableButtons(usdtObject);
        const hint = document.getElementById("city-hint");
        hint.style.visibility = "hidden";
      } else {
        const nonUsdtObject = currentContracts.find(
          (obj) => obj.contractType === first_page_json.contractType
        );
        disableButtons(nonUsdtObject);
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
    first_page_json.contractDuration = Number(event.target.value);
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
  if (isAllFieldsFilled(first_page_json)) {
    console.log(JSON.stringify(first_page_json));
    // отправить хттп запрос с json-файлом

    $.post({
      url: "https://websrustonn.ru/api/v1/prog/tag_values",
      dataType: "json",
      contentType: "application/json",
      data: JSON.stringify(first_page_json),
    })
      .done(function (data) {
        first_page_json_response = data;
        localStorage.setItem(
          "first_page_json_response",
          JSON.stringify(first_page_json_response)
        );
        console.log(first_page_json_response); // Вывод данных для проверки
        // переходим на некст страницу
        // window.location.href = "#popup:page2";
        var linkToPopup = document.createElement("a");
        linkToPopup.setAttribute("href", "#popup:page2");
        linkToPopup.style.display = "none";
        document.querySelector(".r").appendChild(linkToPopup);
        linkToPopup.click();

        let popUpOpn = document.querySelector(".t-popup_show .t-popup__close");
        if (popUpOpn) {
          popUpOpn.click();
        }
        second_page(first_page_json_response);
      })
      .fail(function () {
        alert("Can't load contract");
      });
  }
}

disableButtons(
  currentContracts.find(
    (contract) => contract.contractType === "RUSTONN_PHYS_PERSON"
  )
);
setToDefault();
contractHint();
contractDur();
currency();
money();

let first_page_json_response = JSON.parse(
  localStorage.getItem("first_page_json_response")
);

function second_page(first_page_json_response) {
  // регексы для тегов
  var inputRegex = new Map([
    ["@<DATE>@", /^(0[1-9]|[12][0-9]|3[01])\.(0[1-9]|1[012])\.\d{4}$/],
    ["@<PASSPORT_DATE>@", /^(0[1-9]|[12][0-9]|3[01])\.(0[1-9]|1[012])\.\d{4}$/],
    ["@<BIRTH_DATE>@", /^(0[1-9]|[12][0-9]|3[01])\.(0[1-9]|1[012])\.\d{4}$/],
  ]);

  function getObjectByTag(tag) {
    switch (tag) {
      // text type
      case "@<DATE>@":
      case "@<PASSPORT_DATE>@":
      case "@<BIRTH_DATE>@":
      case "@<BANK_NAME>@":
      case "@<ADDRESS>@":
      case "@<PASSPORT_WHOM>@":
      case "@<FIO_FULL>@":
      case "@<SUMM_NUMBER>@":
      case "@<PASSPORT_SERIA_NUM>@":
        return "text";
        break;
      // number type
      case "@<BIK>@":
      case "@<CORR_ACCOUNT>@":
      case "@<PHONE_NUMBER>@":
      case "@<PAYMENT_ACCOUNT>@":
      case "@<INN>@":
        return "number";
        break;
      // email type
      case "@<EMAIL>@":
        return "email";
        break;
    }
  }

  // добавить элемент для ввода
  function appendChild() {
    console.log(first_page_json_response);
    container = document.getElementById("doc_input");
    const children = container.querySelectorAll("*"); // selects all child elements

    for (let child of children) {
      if (container.contains(child)) {
        container.removeChild(child);
      }
    }

    var jsonArray = Array.from(first_page_json_response.tag_values);
    console.log(jsonArray.length);

    // we don't need to add EXTRA_FIELD type
    for (let i = 0; i < jsonArray.length - 1; i++) {
      var obj = jsonArray[i];
      if (
        obj.tag === "@<DATE>@" ||
        obj.tag === "@<PASSPORT_DATE>@" ||
        obj.tag === "@<BIRTH_DATE>@" ||
        obj.tag === "@<FIO_FULL>@" ||
        obj.tag === "@<SUMM_NUMBER>@"
      ) {
        obj.value = "";
      }
      // Get field name
      value = obj.value;
      // Get field tag
      tag = getObjectByTag(obj.tag);
      // Get placeholder text
      key = obj.key;
      if (tag == "number") {
        container.insertAdjacentHTML(
          "beforeend",
          `<input class="form-element" id= "${obj.tag}" type="${tag}" placeholder="${key}" inputmode="numeric">`
        );
      } else {
        container.insertAdjacentHTML(
          "beforeend",
          `<input class="form-element" id= "${obj.tag}" type="${tag}" placeholder="${key}">`
        );
      }

      console.log(getObjectByTag(obj.tag));
    }

    container.insertAdjacentHTML(
      "beforeend",
      `<div><textarea id="comments" rows="5" cols="50" placeholder="Поле для дополнительной информации и вопросов">`
    );

    try {
      var previousValue = document.getElementById("@<DATE>@").value;
      document
        .getElementById("@<DATE>@")
        .addEventListener("keyup", function () {
          var value = this.value;
          var parts = value.split(".");

          if (value.length < previousValue.length) {
            // символ был удален, не добавляем точку
            previousValue = value;
            return;
          }

          if (parts.length === 1 && value.length === 2) {
            this.value = value + ".";
          } else if (parts.length === 2 && parts[1].length === 2) {
            this.value = value + ".";
          }

          previousValue = this.value;
        });

      var previousValue = document.getElementById("@<BIRTH_DATE>@").value;
      document
        .getElementById("@<BIRTH_DATE>@")
        .addEventListener("keyup", function () {
          var value = this.value;
          var parts = value.split(".");

          if (value.length < previousValue.length) {
            // символ был удален, не добавляем точку
            previousValue = value;
            return;
          }

          if (parts.length === 1 && value.length === 2) {
            this.value = value + ".";
          } else if (parts.length === 2 && parts[1].length === 2) {
            this.value = value + ".";
          }

          previousValue = this.value;
        });

      var previousValue = document.getElementById("@<PASSPORT_DATE>@").value;
      document
        .getElementById("@<PASSPORT_DATE>@")
        .addEventListener("keyup", function () {
          var value = this.value;
          var parts = value.split(".");

          if (value.length < previousValue.length) {
            // символ был удален, не добавляем точку
            previousValue = value;
            return;
          }

          if (parts.length === 1 && value.length === 2) {
            this.value = value + ".";
          } else if (parts.length === 2 && parts[1].length === 2) {
            this.value = value + ".";
          }

          previousValue = this.value;
        });

      var prevSer = document.getElementById("@<PASSPORT_SERIA_NUM>@").value;
      document
        .getElementById("@<PASSPORT_SERIA_NUM>@")
        .addEventListener("keyup", function () {
          var value = this.value;
          var parts = value.split(" ");

          if (value.length < previousValue.length) {
            // символ был удален, не добавляем точку
            previousValue = value;
            return;
          }

          if (parts.length === 1 && value.length === 2) {
            this.value = value + " ";
          } else if (parts.length === 2 && parts[1].length === 2) {
            this.value = value + " ";
          }

          previousValue = this.value;
        });
    } catch (e) {
      console.error("An error occurred with the DATE field: ", e);
    }

    function isValidDate(date) {
      var matches = /^(0[1-9]|[12][0-9]|3[01])\.(0[1-9]|1[012])\.\d{4}$/.exec(
        date
      );
      if (matches == null) return false;
      var d = matches[1];
      var m = matches[2] - 1; // months are 0-based in JavaScript
      var y = matches[3];
      var composedDate = new Date(y, m, d);
      return (
        composedDate.getDate() == d &&
        composedDate.getMonth() == m &&
        composedDate.getFullYear() == y
      );
    }

    // Добавьте обработчик событий к input
    var inputs = document.querySelectorAll(".form-element");
    inputs.forEach(function (input) {
      input.addEventListener("input", function (event) {
        console.log("Input value was changed");
        console.log("New input value: ", event.target.value);
        console.log(event.target.id);
        var tagToFind = event.target.id;
        var indexToFind = jsonArray.findIndex(function (element) {
          return element.tag === tagToFind;
        });
        if (indexToFind === -1) return;

        var value = event.target.value;
        if (
          tagToFind === "@<DATE>@" ||
          tagToFind === "@<PASSPORT_DATE>@" ||
          tagToFind === "@<BIRTH_DATE>@"
        ) {
          if (isValidDate(value)) {
            var dateValue = new Date(value.split(".").reverse().join("-"));
            value = dateValue.toLocaleDateString("ru-RU");
          } else {
            console.log("Invalid date format");
          }
        }

        if (
          tagToFind === "@<FIO_FULL>@" &&
          !/^[\wа-яА-Я]+(\s+[\wа-яА-Я]+)+$/.test(value)
        ) {
          console.error("Введите 2 или более слова, состоящих только из букв");
          event.target.classList.add("invalid-input");
          jsonArray[indexToFind].value = "";
        } else if (tagToFind === "@<PASSPORT_SERIA_NUM>@") {
          let newValue = value.replace(/\D/g, "");
          if (!/^\d{10}$/.test(newValue)) {
            console.error(
              "Введите серию и номер паспорта в формате 1234567890"
            );
            event.target.classList.add("invalid-input");
            jsonArray[indexToFind].value = "";
          } else {
            console.log(value);
            console.log(newValue);
            event.target.classList.remove("invalid-input");
            jsonArray[indexToFind].value = parseInt(newValue);
          }
        } else if (tagToFind === "@<SUMM_NUMBER>@" && !/\d/.test(value)) {
          console.error(
            "Поле с тегом @<SUMM_NUMBER>@ должно содержать хотя бы одну цифру"
          );
          event.target.classList.add("invalid-input");
          jsonArray[indexToFind].value = "";
        } else if (
          (tagToFind === "@<DATE>@" ||
            tagToFind === "@<PASSPORT_DATE>@" ||
            tagToFind === "@<BIRTH_DATE>@") &&
          !/^(0[1-9]|[12][0-9]|3[01])\.(0[1-9]|1[012])\.\d{4}$/.test(value)
        ) {
          console.error(
            "Поле с тегом DATE должно быть корректной датой в формате DD.MM.YEAR"
          );
          event.target.classList.add("invalid-input");
          jsonArray[indexToFind].value = "";
        } else if (tagToFind === "@<SUMM_NUMBER>@") {
          let formatter = new Intl.NumberFormat("ru-RU");
          let newValue = value.replace(/\D/g, "");
          let formattedNumber = formatter.format(newValue); // Outputs: "1 000 000"
          jsonArray[indexToFind].value = parseInt(newValue);
          event.target.value = formattedNumber; // Update the input field value
        } else {
          event.target.classList.remove("invalid-input");
          jsonArray[indexToFind].value = value;
        }

        console.log("Object tag was changed:", jsonArray[indexToFind]);
      });
    });

    // Добавьте обработчик событий к textarea
    var textarea = document.getElementById("comments");
    textarea.addEventListener("input", function (event) {
      console.log("Textarea value was changed");
      console.log("New textarea value: ", event.target.value);
      console.log(event.target.id);
      var keyToFind = "Поле для дополнительной информации и вопросов";
      var indexToFind = jsonArray.findIndex(function (element) {
        return element.key === keyToFind;
      });
      if (indexToFind !== -1) {
        jsonArray[indexToFind].value = event.target.value;
        console.log("Object key was changed:", jsonArray[indexToFind]);
      }
    });
  }

  function finalReq() {
    var shouldSendRequest = true;
    prevButton = document.getElementById("prev_button");
    prev_button.addEventListener("click", function (event) {
      console.log("prev_button");
      var linkToPopup = document.createElement("a");
      linkToPopup.setAttribute("href", "#popup:page1"); // Измените на #popup:page1
      linkToPopup.style.display = "none";
      document.querySelector(".r").appendChild(linkToPopup);
      linkToPopup.click();

      var element = document.getElementsByClassName(
        "t-popup__close-wrapper t-popup__block-close-button"
      )[1];

      // Клик по элементу
      element.click();
    });

    finalBut = document.getElementById("final_button");
    finalBut.addEventListener("click", function (event) {
      // отправить финальный реквест с json файлом и токеном
      // отправить хттп запрос с json-файлом
      console.log("token", `Bearer ${first_page_json_response.token}`);
      console.log(
        "tag_values",
        JSON.stringify(first_page_json_response.tag_values)
      );

      // Проверка значений полей перед отправкой запроса
      var tagsToCheck = [
        "@<DATE>@",
        "@<PASSPORT_DATE>@",
        "@<BIRTH_DATE>@",
        "@<FIO_FULL>@",
        "@<SUMM_NUMBER>@",
      ];
      for (let i = 0; i < tagsToCheck.length; i++) {
        var tag = tagsToCheck[i];
        var index = first_page_json_response.tag_values.findIndex(function (
          element
        ) {
          return element.tag === tag;
        });
        if (
          index !== -1 &&
          first_page_json_response.tag_values[index].value === ""
        ) {
          console.error(`Поле с тегом ${tag} не должно быть пустым`);
          // Подчеркнуть поле красным цветом
          var input = document.getElementById(tag);
          if (input) {
            input.style.border = "1px solid red";
          }
          shouldSendRequest = false;
          return;
        }
      }
      if (shouldSendRequest) {
        $.post({
          url: "https://websrustonn.ru/api/v1/prog/filled_content",
          type: "POST",
          dataType: "json",
          contentType: "application/json",
          data: JSON.stringify({
            from_tag_values: first_page_json_response.tag_values,
          }),
          headers: {
            Authorization: `Bearer ${first_page_json_response.token}`,
          },
        })
          .done((data) => {
            console.log(data);
            var file_name = data.fileName;
            // Выполнить GET запрос сразу после получения ответа на POST запрос
            $.ajax({
              url: `https://websrustonn.ru/${data.fileName}`, // replace with your URL
              type: "GET",
              xhrFields: {
                responseType: "blob", // to avoid binary data being mangled on charset conversion
              },
              success: function (blob) {
                // Create URL from Blob
                const url = window.URL.createObjectURL(blob);
                // Create download link
                const a = document.createElement("a");
                a.href = url;
                var parts = file_name.split("/");
                // Set the download file name
                a.download = parts[2];
                // Trigger the download
                document.body.appendChild(a);
                a.click();
                // Remove the temporary link
                document.body.removeChild(a);
                var element = document.getElementsByClassName(
                "t-popup__close-wrapper t-popup__block-close-button"
                )[1];

                // Клик по элементу
                element.click();
                location.reload();
              },
              error: function (err) {
                console.error("Ошибка:", err);
              },
            });
          })
          .fail((error) => {
            console.error("Error:", error);
          });
      }
    });
  }

  finalReq();
  appendChild();
}
