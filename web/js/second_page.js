let first_page_json = JSON.parse(
  '[{"key":"Введите дату","tag":"@<DATE>@","value":"Дата заполнения"},{"key":"Введите ФИО","tag":"@<FIO_FULL>@","value":"ФИО займодавца"},{"key":"Введите сумму займа","tag":"@<SUMM_NUMBER>@","value":"Сумма займа числом"},{"key":"Введите Серию и Номер паспорта","tag":"@<PASSPORT_SERIA_NUM>@","value":"Серия и номер паспорта"},{"key":"Введите кем и когда выдан паспорт","tag":"@<PASSPORT_WHOM>@","value":"Кем и когда выдан паспорт"},{"key":"Введите дату выдачи паспорта","tag":"@<PASSPORT_DATE>@","value":"Дата выдачи паспорта"},{"key":"Введите адрес регистрации","tag":"@<ADDRESS>@","value":"Адрес регистрации займодавца"},{"key":"Введите ИНН","tag":"@<INN>@","value":"ИНН займодавца"},{"key":"Введите название банка (для расчетов)","tag":"@<BANK_NAME>@","value":"Наименование банка"},{"key":"Введите расчётный счёт","tag":"@<PAYMENT_ACCOUNT>@","value":"Расчётный счёт"},{"key":"Введите БИК","tag":"@<BIK>@","value":"БИК"},{"key":"Введите корр. счёт","tag":"@<CORR_ACCOUNT>@","value":"Корр. счёт"},{"key":"Введите номер телефона","tag":"@<PHONE_NUMBER>@","value":"Номер телефона займодавца"},{"key":"Введите электронную почту займодавца","tag":"@<EMAIL>@","value":"Электронная почта займодавца"},{"key":"","tag":"@<EXTRA_FIELD>@","value":"Доп. поле"}]'
);

function getObjectByTag(tag) {
  switch (tag) {
    // data type
    case "@<DATE>@":
    case "@<PASSPORT_DATE>@":
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
  container = document.getElementById("doc_input");
  var jsonArray = Array.from(first_page_json);
  console.log(jsonArray.length);
  // we don't need to add EXTRA_FIELD type
  for (let i = 0; i < jsonArray.length - 1; i++) {
    var obj = jsonArray[i];
    // Get field name
    value = obj.value;
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
      `<input class="form-element" id= "input" type="${tag}" placeholder="${key}">`
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
}

function onPrevPage()
{
  console.log(first_page_json.contractType + " " + first_page_json.currencyType + " " + first_page_json.currencyKind);
  // отправить хттп запрос с json-файлом

  window.location.href = "first_page.html"
}

appendChild();
// здесь нужны функции отправки запросов и принятия
//@todo добавить кнопки отправить и назад, токен-хуекен
// @todo добавить функции, которые будут сохранять жсон
