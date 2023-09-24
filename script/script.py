import docx
import sys
import re
import json
import os
import logging

from docxcompose.composer import Composer
from docx import Document as Document_compose
from num_to_rus import Converter

import datetime
from dateutil.relativedelta import *

# argv[1] - строка шаблона json
# argv[2] - путь для сохранения файла

def space_num(num):
    return format(num, ',d').replace(',',' ')

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

def make_tables(document, type, tag_dict):
        logging.debug(f'table type with num={type}')
        if type <= 4:
                make_table_1(document, tag_dict)
        elif type <= 9:
                make_table_2(document, tag_dict)
        elif type <= 10:
                make_table_3(document, tag_dict, 'долларов США')
        elif type <= 11:
                make_table_3(document, tag_dict, 'Евро')

def make_table_1(document, tag_dict):
        logging.debug('make_table_1')
        date = datetime.datetime.strptime(tag_dict['@<DATE>@'], '%d.%m.%Y')
        loan_sum = int(tag_dict['@<SUMM_NUMBER>@'].replace(' ', ''))

        payment = loan_sum * int(tag_dict['@<PERCENT_NUMBER>@']) // 1200

        hold_sum = payment * 13 // 100
        if (payment * 13) % 100 > 0:
                hold_sum = hold_sum + 1

        result_sum = payment - hold_sum

        # проверка на нужную таблицу
        for table in document.tables:
                if table.rows[0].cells[0].text  == '№ п/п':
                        fill_cell_date(table, 1, date) # дата погашения
                        fill_cell_table(table, 2, space_num(payment)) # сумма платежа
                        fill_cell_table(table, 3, space_num(hold_sum)) # удерживаемая сумма налога
                        fill_cell_table(table, 4, space_num(result_sum)) # итоговая сумма выплаты Займодавцу
                        table.columns[5].cells[13].paragraphs[0].runs[0].text = space_num(loan_sum) + ' (' + str(Converter().convert(loan_sum)) + ') рублей' # сумма основного долга

                        # Заполнение ИТОГО
                        table.rows[14].cells[2].paragraphs[0].runs[0].text = space_num(payment * 12) + ' (' + str(Converter().convert(payment * 12)) + ') рублей' # сумма платежа
                        table.rows[14].cells[3].paragraphs[0].runs[0].text = space_num(hold_sum * 12) + ' (' + str(Converter().convert(hold_sum * 12)) + ') рублей' # удерживаемая сумма налога
                        table.rows[14].cells[4].paragraphs[0].runs[0].text = space_num(result_sum * 12) + ' (' + str(Converter().convert(result_sum * 12)) + ') рублей' # утоговая сумма выплаты Займодавцу
                        table.rows[14].cells[5].paragraphs[0].runs[0].text = space_num(loan_sum) + ' (' + str(Converter().convert(loan_sum)) + ') рублей' # сумма основного долга

def make_table_2(document, tag_dict):
        logging.debug('make_table_2')
        date = datetime.datetime.strptime(tag_dict['@<DATE>@'], '%d.%m.%Y')
        loan_sum = int(tag_dict['@<SUMM_NUMBER>@'].replace(' ', ''))

        payment = loan_sum * int(tag_dict['@<PERCENT_NUMBER>@']) // 1200

        # проверка на нужную таблицу
        for table in document.tables:
                if table.rows[0].cells[0].text  == '№ п/п':
                        table.rows[0].cells[2].paragraphs[0].runs[0].text = 'Сумма процентов: ' + str(perc) + '%'

                        fill_cell_date(table, 1, date) # дата погашения
                        fill_cell_table(table, 2, space_num(payment)) # сумма платежа

                        table.columns[3].cells[13].paragraphs[0].runs[0].text = space_num(loan_sum) # сумма основного долга

                        # Заполнение ИТОГО
                        table.rows[14].cells[2].paragraphs[0].runs[0].text = space_num(payment * 12) + ' (' + str(Converter().convert(payment * 12)) + ') рублей' # сумма платежа
                        table.rows[14].cells[3].paragraphs[0].runs[0].text = space_num(loan_sum) + ' (' + str(Converter().convert(loan_sum)) + ') рублей' # сумма основного долга

def make_table_3(document, tag_dict, currency):
        logging.debug('make_table_3')
        date = datetime.datetime.strptime(tag_dict['@<DATE>@'], '%d.%m.%Y')
        loan_sum = int(tag_dict['@<SUMM_NUMBER>@'].replace(' ', ''))

        payment = loan_sum * int(tag_dict['@<PERCENT_NUMBER>@']) // 1200

        # проверка на нужную таблицу
        for table in document.tables:
                if table.rows[0].cells[0].text == '№':

                        fill_cell_date(table, 1, date, 0) # дата погашения
                        fill_cell_table(table, 2, space_num(payment) + ' ' + currency + ' в российских рублях по курсу ЦБ РФ на день выплаты', 0) # сумма платежа

                        # заполнение ИТОГО
                        table.rows[12].cells[2].paragraphs[0].runs[0].text = 'Возвращается Сумма займа в размере ' + space_num(payment * 12) + ' (' + str(Converter().convert(payment * 12)) + ') ' + currency # сумма платежа

def get_docs_nums(data): # получим номера нужных документов
        doc_nums = []
        for need_docs in data["docs"]:
                doc_nums.append(int(need_docs))
        return doc_nums

def get_table_need_values(data):
        for entry in data["tag_values"]:
                if entry["tag"] == "@<SUMM_NUMBER>@":
                        loan_sum = int(entry["key"])
                elif entry["tag"] == "@<PERCENT_NUMBER>@":
                        perc = int(entry["key"])
                elif entry["tag"] == "@<DATE>@":
                        date = entry["key"]
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

def correct_values(tag_dict):
        if '@<SUMM_NUMBER>@' in tag_dict:
                tag_dict['@<SUMM_NUMBER>@'] = space_num(int(tag_dict['@<SUMM_NUMBER>@'])) # если это сумма договора, то разделяем пробелами

def replace_tags(document, tag_dict):
        for tag, value in tag_dict.items(): # бежим по тэгам
                regex = re.compile(tag)
                for paragraph in document.paragraphs:
                        paragraph_replace_text(paragraph, regex, value)
                tables_replace_text(document, regex, value)

def make_fio_short(fio_full):
        return fio_full.split()[0] + ' ' + fio_full.split()[1][0] + '. ' + fio_full.split()[2][0] + '.'

def split_passport(seria_num):
        if len(seria_num.split()) == 1:
                return seria_num[0:4], seria_num[4:]
        return seria_num.split()[0], seria_num.split()[1]

def make_contract_num(tag_dict):
        date = datetime.datetime.strptime(tag_dict['@<DATE>@'], '%d.%m.%Y')
        datestr = date.strftime('%d%m%y')
        return datestr + tag_dict['@<FIO_FULL>@'].split()[0][0] + tag_dict['@<FIO_FULL>@'].split()[1][0] + tag_dict['@<FIO_FULL>@'].split()[2][0]

def add_extra_tags(tag_dict):
        if '@<SUMM_NUMBER>@' in tag_dict:
                tag_dict['@<SUMM_TEXT>@'] = str(Converter().convert(int(tag_dict['@<SUMM_NUMBER>@'])))
        if '@<PERCENT_NUMBER>@' in tag_dict:
                tag_dict['@<PERCENT_TEXT>@'] = str(Converter().convert(int(tag_dict['@<PERCENT_NUMBER>@'])))
        if '@<FIO_FULL>@' in tag_dict:
                tag_dict['@<FIO_SHORT>@'] = make_fio_short(tag_dict['@<FIO_FULL>@'])
        if '@<PASSPORT_SERIA_NUM>@' in tag_dict:
                tag_dict['@<PASSPORT_SERIA>@'], tag_dict['@<PASSPORT_NUM>@'] = split_passport(tag_dict['@<PASSPORT_SERIA_NUM>@'])
        tag_dict['@<GUARANTEE_NUM>@'] = str(make_contract_num(tag_dict))
        tag_dict['@<CONTRACT_NUM>@'] = str(make_contract_num(tag_dict))

def make_fio_date(tag_dict):
        return tag_dict['@<DATE>@'] + ' ' + tag_dict['@<FIO_SHORT>@']

def make_docs():
        data = json.loads(str(sys.argv[1])) # загружаем шаблон
        data = data['contracts'][0] # встаём на единственный контракт

        tag_dict = {} # создаем словарь соответствия тэгов и значений
        for entry in data['tag_values']: # заполняем словарь тэгов
                tag_dict[entry['tag']] = str(entry['key'])
        add_extra_tags(tag_dict) # добавить новые необходимые теги, такие как текстовые представления чисел
        correct_values(tag_dict) # подмениваем значения по необходимости
        logging.debug(f'tag_dict = : {tag_dict}')

        doc_nums = get_docs_nums(data) # получаем номера документов
        for doc_num in doc_nums: # бежим по номерам документов
                document = docx.Document(DOCX_TEMPLATES_PATH + str(doc_num) + '.docx') # открываем шаблон документа
                replace_tags(document, tag_dict) # заменяем тэги на нужные значения
                make_tables(document, doc_num, tag_dict) # заполняем таблицы
                save_name = str(sys.argv[2] + get_name_by_num(doc_num) + ' от ' + make_fio_date(tag_dict) + ".docx")
                logging.debug(f"Name to save is {save_name}")
                document.save(save_name)

if __name__ == "__main__":
        LOG_FULL_NAME = 'python_script.log'
        DOCX_TEMPLATES_PATH = '/app/templates/docx/'

        logging.basicConfig(level=logging.DEBUG, filename=LOG_FULL_NAME) # запись логов уровня дебаг в файл LOG_FULL_NAME
        logging.debug(f'Args length: {len(sys.argv)}')
        logging.debug(f'Args = {str(sys.argv)}')

        make_docs()
