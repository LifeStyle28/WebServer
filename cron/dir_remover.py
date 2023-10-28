# папка result должна содержать только сгенерированные папки!
import re
import subprocess
import os
from datetime import datetime
from datetime import timedelta

for file in os.listdir(r'/app/web/result'):
	command = r'stat /app/web/result/' + file + r' | grep Access:\ 2'
	time_str = subprocess.getoutput(command)
	match = re.search(r'(\d\d\d\d-\d\d-\d\d) (\d\d:\d\d:\d\d)', time_str)
	last_use_datetime = datetime.fromisoformat(str(match[1]) + ' ' + str(match[2]));

	if (datetime.today() - last_use_datetime) > timedelta(hours=1):
		print('rm -rf ' + file)
	else:
		print(file, ' left')
