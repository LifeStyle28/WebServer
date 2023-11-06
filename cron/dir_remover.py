# папка result должна содержать только сгенерированные папки!
import re
import subprocess
import os
import logging
from datetime import datetime
from datetime import timedelta

logging.basicConfig(level=logging.DEBUG, filename='/cron/dir_remover.log')

for file in os.listdir(r'/result'):
	command = r'stat /result/' + file + r' | grep Access:\ 2'
	time_str = subprocess.getoutput(command)
	match = re.search(r'(\d\d\d\d-\d\d-\d\d) (\d\d:\d\d:\d\d)', time_str)
	last_use_datetime = datetime.fromisoformat(str(match[1]) + ' ' + str(match[2]));

	if (datetime.today() - last_use_datetime) > timedelta(minutes=30):
		logging.debug(f'rm -rf /result/{file}')
		os.system('rm -rf /result/' + file)
	else:
		logging.debug(f'{file} left')
