let first_page_json_response = JSON.parse(
  localStorage.getItem("first_page_json_response")
);

function getObjectByTag(tag) {
  switch (tag) {
    // data type
    case "@<DATE>@":
    case "@<PASSPORT_DATE>@":
    case "@<BIRTH_DATE>@":
      return "date";
      break;
    // text type
    case "@<BANK_NAME>@":
    case "@<ADDRESS>@":
    case "@<PASSPORT_WHOM>@":
    case "@<FIO_FULL>@":
      return "text";
      break;
    // number type
    case "@<BIK>@":
    case "@<CORR_ACCOUNT>@":
    case "@<PHONE_NUMBER>@":
    case "@<PAYMENT_ACCOUNT>@":
    case "@<INN>@":
    case "@<SUMM_NUMBER>@":
    case "@<PASSPORT_SERIA_NUM>@":
      return "number";
      break;
    // email type
    case "@<EMAIL>@":
      return "email";
      break;
  }
}

// добавить элемент для ввода
// @todo добавить формирование json-файла, который мы отошлем
function appendChild() {
  console.log(first_page_json_response);
  container = document.getElementById("doc_input");
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
    // add html object
    container.insertAdjacentHTML(
      "beforeend",
      `<label id = "form_value">${value}</label>`
    );
    // Get field tag
    tag = getObjectByTag(obj.tag);
    // Get placeholder text
    key = obj.key;
    container.insertAdjacentHTML(
      "beforeend",
      `<input class="form-element" id= "${obj.tag}" type="${tag}" placeholder="${key}">`
    );
    console.log(getObjectByTag(obj.tag));
  }

  // add extrafield type
  container.insertAdjacentHTML(
    "beforeend",
    `<label id = "form_value">Дополнительное поле</label>`
  );
  container.insertAdjacentHTML(
    "beforeend",
    `<div><textarea id="comments" rows="5" cols="50">`
  );

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
        var dateValue = new Date(value);
        value = dateValue.toLocaleDateString("ru-RU");
      }

      if (
        tagToFind === "@<FIO_FULL>@" &&
        !/^[\wа-яА-Я]+(\s+[\wа-яА-Я]+)+$/.test(value)
      ) {
        console.error("Введите 2 или более слова, состоящих только из букв");
        event.target.classList.add("invalid-input");
      } else if (
        tagToFind === "@<PASSPORT_SERIA_NUM>@" &&
        !/^\d{2}\d{8}$/.test(value)
      ) {
        console.error("Введите серию и номер паспорта в формате 1234567890");
        event.target.classList.add("invalid-input");
      } else if (tagToFind === "@<SUMM_NUMBER>@" && !/\d/.test(value)) {
        console.error(
          "Поле с тегом @<SUMM_NUMBER>@ должно содержать хотя бы одну цифру"
        );
        event.target.classList.add("invalid-input");
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

function onPrevPage() {
  //console.log(first_page_json.contractType + " " + first_page_json.currencyType + " " + first_page_json.currencyKind);
  // отправить хттп запрос с json-файлом

  window.location.href = "first_page.html";
}

function onFinalReq() {
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
      return;
    }
  }

  fetch("http://localhost/api/v1/prog/filled_content", {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
      Accept: "application/json",
      Authorization: `Bearer ${first_page_json_response.token}`,
    },
    body: JSON.stringify({
      from_tag_values: first_page_json_response.tag_values,
    }),
  })
    .then((response) => response.json())
    .then((data) => {
      console.log(data);

      // Выполнить GET запрос сразу после получения ответа на POST запрос
      fetch(`http://localhost/${data.fileName}`)
        .then((response) => response.blob()) // Декодируем ответ в формате Blob
        .then((data) => {
          // Создаем объект URL из Blob
          const url = window.URL.createObjectURL(data);
          // Создаем ссылку для скачивания
          const a = document.createElement("a");
          a.href = url;
          // Устанавливаем имя файла
          a.download = "docs.zip";
          // Инициируем скачивание
          document.body.appendChild(a);
          a.click();
          // Удаляем временную ссылку
          document.body.removeChild(a);
        })
        .catch((error) => console.error("Ошибка:", error));
    })
    .catch((error) => {
      console.error("Error:", error);
    });
}

appendChild();
// здесь нужны функции отправки запросов и принятия
//@todo добавить кнопки отправить и назад, токен-хуекен
// @todo добавить функции, которые будут сохранять жсон
