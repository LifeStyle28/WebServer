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
      if (indexToFind !== -1) {
        if (tagToFind === "@<DATE>@" || tagToFind === "@<PASSPORT_DATE>@" || tagToFind === "@<BIRTH_DATE>@") {
          var dateValue = new Date(event.target.value);
          var formattedDate = dateValue.toLocaleDateString('ru-RU');
          jsonArray[indexToFind].value = formattedDate;
        } else {
          jsonArray[indexToFind].value = event.target.value;
        }
        console.log("Object tag was changed:", jsonArray[indexToFind]);
      }
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
  console.log("token", "Bearer ${first_page_json_response.token}");
  console.log(
    "tag_values",
    JSON.stringify(first_page_json_response.tag_values)
  );
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
    .then((data) => console.log(data))
    .catch((error) => {
      console.error("Error:", error);
    });
}

appendChild();
// здесь нужны функции отправки запросов и принятия
//@todo добавить кнопки отправить и назад, токен-хуекен
// @todo добавить функции, которые будут сохранять жсон
