import docx
import sys
import re
import json
import os

from docxcompose.composer import Composer
from docx import Document as Document_compose
from num_to_rus import Converter
# from docx2pdf import convert

import datetime
from dateutil.relativedelta import *

# argv[1] - путь до шаблона json
# argv[2] - путь для сохранения файла

# argv[3] - тип сохраняемого документа: 1 - docx, 2 - pdf, 3 - docx and pdf

def space_num(num):
    return format(num, ',d').replace(',',' ')

def combine_all_docx(filename_master, merge_file):
    master = Document_compose(filename_master)
    composer = Composer(master)
    doc_temp = Document_compose(merge_file)
    composer.append(doc_temp)
    composer.save("combined_file.docx")

def paragraph_replace_text(paragraph, regex, replace_str):
    while True:
        text = paragraph.text
        match = regex.search(text)
        if not match:
            break

        runs = iter(paragraph.runs)
        start, end = match.start(), match.end()

        for run in runs:
            run_len = len(run.text)
            if start < run_len:
                break
            start, end = start - run_len, end - run_len

        run_text = run.text
        run_len = len(run_text)
        run.text = "%s%s%s" % (run_text[:start], replace_str, run_text[end:])
        end -= run_len
        for run in runs:
            if end <= 0:
                break
            run_text = run.text
            run_len = len(run_text)
            run.text = run_text[end:]
            end -= run_len
    return paragraph

def fill_cell_table(table, column_num, text, row_set = 1):
        for cell_idx, value in enumerate(table.columns[column_num].cells): # Дата погашения
                                if not(cell_idx > row_set and (cell_idx < 13 + row_set)):
                                        continue
                                value.paragraphs[0].runs[0].text = text

def fill_cell_date(table, column_num, use_date, row_set = 1):
        for cell_idx, value in enumerate(table.columns[column_num].cells): # Дата погашения
                if not(cell_idx > row_set and (cell_idx < 13 + row_set)):
                        continue
                use_date = use_date + relativedelta(months=+1)
                #cur_date = str(use_date.day) + '.' + str(use_date.month) + '.' + str(use_date.year)
                cur_date = use_date.strftime("%d.%m.%Y")
                value.paragraphs[0].runs[0].text = cur_date

def make_tables(document, type, loan_sum, perc, date):
        print("table type with num=", type)
        if type <= 4:
                make_table_1(document, loan_sum, perc, date)
        elif type <= 9:
                make_table_2(document, loan_sum, perc, date)
        elif type <= 10:
                make_table_3(document, loan_sum, perc, date, 'долларов США')
        elif type <= 11:
                make_table_3(document, loan_sum, perc, date, 'Евро')

def make_table_1(document, loan_sum, perc, date):
        print("make_table_1")
        conv = Converter()
        date = datetime.datetime.strptime(date, '%d.%m.%Y')

        payment = loan_sum * perc // 1200

        hold_sum = payment * 13 // 100
        if (payment * 13) % 100 > 0:
                hold_sum = hold_sum + 1

        result_sum = payment - hold_sum

        #Проверка на нужную таблицу
        for table in document.tables:
                if table.rows[0].cells[0].text  == '№ п/п':
                        fill_cell_date(table, 1, date) # Дата погашения
                        fill_cell_table(table, 2, space_num(payment)) # Сумма платежа
                        fill_cell_table(table, 3, space_num(hold_sum)) # Удерживаемая сумма налога
                        fill_cell_table(table, 4, space_num(result_sum)) # Итоговая сумма выплаты Займодавцу
                        table.columns[5].cells[13].paragraphs[0].runs[0].text = space_num(loan_sum) + ' (' + str(conv.convert(loan_sum)) + ') рублей' # Сумма основного долга

                        # Заполнение ИТОГО
                        table.rows[14].cells[2].paragraphs[0].runs[0].text = space_num(payment * 12) + ' (' + str(conv.convert(payment * 12)) + ') рублей' # Сумма платежа
                        table.rows[14].cells[3].paragraphs[0].runs[0].text = space_num(hold_sum * 12) + ' (' + str(conv.convert(hold_sum * 12)) + ') рублей' # Удерживаемая сумма налога
                        table.rows[14].cells[4].paragraphs[0].runs[0].text = space_num(result_sum * 12) + ' (' + str(conv.convert(result_sum * 12)) + ') рублей' # Итоговая сумма выплаты Займодавцу
                        table.rows[14].cells[5].paragraphs[0].runs[0].text = space_num(loan_sum) + ' (' + str(conv.convert(loan_sum)) + ') рублей' # Сумма основного долга

def make_table_2(document, loan_sum, perc, date):
        print("make_table_2")
        conv = Converter()
        date = datetime.datetime.strptime(date, '%d.%m.%Y')

        payment = loan_sum * perc // 1200

        #Проверка на нужную таблицу
        for table in document.tables:
                if table.rows[0].cells[0].text  == '№ п/п':
                        table.rows[0].cells[2].paragraphs[0].runs[0].text = 'Сумма процентов: ' + str(perc) + '%'

                        fill_cell_date(table, 1, date) # Дата погашения
                        fill_cell_table(table, 2, space_num(payment)) # Сумма платежа

                        table.columns[3].cells[13].paragraphs[0].runs[0].text = space_num(loan_sum) # Сумма основного долга

                        # Заполнение ИТОГО
                        table.rows[14].cells[2].paragraphs[0].runs[0].text = space_num(payment * 12) + ' (' + str(conv.convert(payment * 12)) + ') рублей' # Сумма платежа
                        table.rows[14].cells[3].paragraphs[0].runs[0].text = space_num(loan_sum) + ' (' + str(conv.convert(loan_sum)) + ') рублей' # Сумма основного долга

def make_table_3(document, loan_sum, perc, date, currency):
        print("make_table_3")
        conv = Converter()
        date = datetime.datetime.strptime(date, '%d.%m.%Y')

        payment = loan_sum * perc // 1200

        #Проверка на нужную таблицу
        for table in document.tables:
                if table.rows[0].cells[0].text == '№':

                        fill_cell_date(table, 1, date, 0) # Дата погашения
                        fill_cell_table(table, 2, space_num(payment) + ' ' + currency + ' в российских рублях по курсу ЦБ РФ на день выплаты', 0) # Сумма платежа

                        # Заполнение ИТОГО
                        table.rows[12].cells[2].paragraphs[0].runs[0].text = 'Возвращается Сумма займа в размере ' + space_num(payment * 12) + ' (' + str(conv.convert(payment * 12)) + ') ' + currency # Сумма платежа

def get_docs_nums(data): # Получим номера нужных документов
        doc_nums = []
        for need_docs in data["docs"]:
                doc_nums.append(int(need_docs))
        return doc_nums

def get_table_need_values(data):
        for entry in data["tag_values"]:
                if entry["tag"] == "@<SUMM_NUMBER>@":
                        loan_sum = int(entry["def_value"])
                elif entry["tag"] == "@<PERCENT_NUMBER>@":
                        perc = int(entry["def_value"])
                elif entry["tag"] == "@<DATE>@":
                        date = entry["def_value"]
        return loan_sum, perc, date

def tables_replace_text(document, regex, new_value):
        for table in document.tables:
                for row in table.rows:
                        for cell in row.cells:
                                for paragraph in cell.paragraphs:
                                        paragraph_replace_text(paragraph, regex, new_value)
def get_name_by_num(num):
    file_names = {1: 'Рустонн + физ. лицо (Руб)',
                  5: 'Рустонн + юр. лицо (Руб)',
                  9: 'Пугачев Т.В. + физ. лицо (Руб)',
                  10: 'Пугачев Т.В. + физ. лицо ($)',
                  11: 'Пугачев Т.В. + физ. лицо (Euro)',
                  13: 'Поручительство Пугачев Т.В. (Руб)',
                  17: 'Поручительство Рустонн (Руб)',
                  18: 'Поручительство Рустонн ($)',
                  19: 'Поручительство Рустонн (Euro)',
                  21: 'Расписка (Руб)',
                  22: 'Расписка ($)',
                  23: 'Расписка (Euro)'}
    return file_names[num]

if __name__ == "__main__":
        print("Args length:", len(sys.argv))
        print("Arg[1] =", str(sys.argv[1]))
        print("Arg[2] =", str(sys.argv[2]))
        #print("Arg[3] =", int(sys.argv[3]))

        error_status = 0
        with open(str(sys.argv[1]), encoding="utf8") as json_file: # открываем json-шаблон
                data = json.load(json_file)
        doc_nums = get_docs_nums(data)

        for i in doc_nums:
                doc_num = int(i)
                document = docx.Document("docx/" + str(doc_num) + ".docx")

                for entry in data["tag_values"]:
                        for curr in entry:
                                if curr == "tag":
                                        regex = re.compile(entry[curr])
                                        if entry[curr] == "@<SUMM_NUMBER>@":
                                            new_value = space_num(int(new_value)) # если это сумма договора, то разделяем пробелами
                                elif curr == "def_value":
                                        new_value = entry[curr]
                        for paragraph in document.paragraphs:
                                paragraph_replace_text(paragraph, regex, new_value)
                        tables_replace_text(document, regex, new_value)

                loan_sum, perc, date = get_table_need_values(data)
                make_tables(document, doc_num, loan_sum, perc, date)

                # if int(sys.argv[3]) == 2:
                #     save_name = str("temp/" + get_name_by_num(doc_num) + ".docx")
                # else:
                save_name = str(sys.argv[2] + get_name_by_num(doc_num) + ".docx")
                print("Name to save is", save_name)
                document.save(save_name)
                # try:
                #         if int(sys.argv[3]) > 1:
                #                 convert(save_name, str(sys.argv[2] + get_name_by_num(doc_num) + ".pdf"))
                # except:
                #         print("Error converting to PDF, file #", doc_num)
                #         error_status = 1
                # if int(sys.argv[3]) == 2: # если вариант "только PDF", то удаляем docx
                #         os.remove(save_name)
        print("Program exit. Error status is", error_status)
        if error_status == 1:
            exit(int(error_status))
