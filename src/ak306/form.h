static const unsigned char form[]="[{\"type\":\"group\",\"label\":\"Дополнительно\",\"items\":[{\"type\":\"select\",\"label\":\"Прошивка\",\"id\":\"fw\",\"size\":40"
	",\"value\":\"fw_value***********************\",\"items\":[{\"FIRMWARE\":0}]},{\"type\":\"select\",\"label\":\"Определение движения\""
	",\"id\":\"move\",\"value\":\"move_value\",\"size\":40,\"items\":[{\"id\":0,\"label\":\"По зажиганию\"},{\"id\":1,\"label\":\"По датчику скорости №1\""
	"},{\"id\":2,\"label\":\"По датчику скорости №2\"},{\"id\":3,\"label\":\"По датчику скорости №3\""
	"},{\"id\":4,\"label\":\"По датчику скорости №4\"}]},{\"type\":\"text\",\"label\":\"Время без импульсов после которого начинается стоянка\""
	",\"id\":\"park_time\",\"valuetype\":\"numeric\",\"value\":\"park_time_value\",\"minvalue\":1,\"maxvalue\":3600},{\"type\":\"text\",\"label\":\"Число импульсов после которого начинается движение\""
	",\"id\":\"impulse_count\",\"valuetype\":\"numeric\",\"value\":\"impulse_count_value\",\"minvalue\":1,\"maxvalue\":3600}]},{\"type\":\"condition\""
	",\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"move\"},\"operand2\":0},\"actions\":[{\"selector\":true,\"command\":\"show\",\"item\":\"park_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"impulse_count\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"park_time\"},{\"selector\":false"
	",\"command\":\"hide\",\"item\":\"impulse_count\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// ACTIVE MODE                                                \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Активный режим\",\"items\":[{\"type\":\"text\",\"label\":\"Интервал замера координат (сек)\""
	",\"id\":\"active_interval\",\"valuetype\":\"numeric\",\"value\":\"active_interval_value\",\"minvalue\":1,\"maxvalue\":3600},{\"type\":\"text\""
	",\"label\":\"Количество точек в посылке\",\"id\":\"active_ppp\",\"minvalue\":1,\"maxvalue\":4"
	",\"value\":\"active_ppp_value\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// WAIT MODE                                                  \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Ждущий режим\",\"id\":\"wait_enable\",\"collapsible\":true,\"expanded\":\"wait_expanded\""
	",\"items\":[{\"type\":\"text\",\"label\":\"Задержка перехода в ждущий режим после выключения зажигания (минуты)\""
	",\"id\":\"wait_delay\",\"valuetype\":\"numeric\",\"value\":\"wait_delay_value\",\"minvalue\":0,\"maxvalue\":9999},{\"type\":\"text\",\"label\":\"Интервал замера координат (минуты) [1..1440]\""
	",\"id\":\"wait_interval\",\"valuetype\":\"numeric\",\"value\":\"wait_interval_value\",\"minvalue\":1,\"maxvalue\":1440}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// SLEEP MODE                                                 \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Спящий режим\",\"id\":\"sleep_enable\",\"collapsible\":true,\"expanded\":\"sleep_expanded\""
	",\"items\":[{\"type\":\"text\",\"label\":\"Задержка перехода в спящий режим после выключения зажигания (минуты)\""
	",\"id\":\"sleep_delay\",\"valuetype\":\"numeric\",\"value\":\"sleep_delay_value\",\"minvalue\":0,\"maxvalue\":9999},{\"type\":\"text\",\"label\":\"Интервал замера координат (часы) [1..24]\""
	",\"id\":\"sleep_interval\",\"valuetype\":\"numeric\",\"value\":\"sleep_interval_value\",\"minvalue\":1,\"maxvalue\":24},{\"type\":\"text\""
	",\"label\":\"Максимальное время поиска GPS сигнала (минуты)\""
	",\"id\":\"sleep_max_nav_search\",\"valuetype\":\"numeric\",\"value\":\"sleep_nav_value\",\"minvalue\":0,\"maxvalue\":9999},{\"type\":\"text\""
	",\"label\":\"Максимальное время поиска GSM сигнала (минуты)\""
	",\"id\":\"sleep_max_gsm_search\",\"valuetype\":\"numeric\",\"value\":\"sleep_gsm_value\",\"minvalue\":0,\"maxvalue\":9999}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// INPUT 1                                                    \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Дискретный вход 1\",\"id\":\"input1\",\"collapsible\":true,\"expanded\":\"input1_expanded\""
	",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\",\"id\":\"input1_type\",\"value\":\"input1_type_value\",\"size\":40"
	",\"items\":[{\"id\":0,\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Зажигание\"},{\"id\":3,\"label\":\"Дискретный датчик\""
	"},{\"id\":4,\"label\":\"Импульсный датчик\"},{\"id\":11,\"label\":\"Кнопка вызова диспетчера\""
	"}]},{\"type\":\"text\",\"label\":\"Минимальная длительность для активации (мс)\""
	",\"id\":\"input1_min_active_time\",\"valuetype\":\"numeric\",\"value\":\"input1_min_active_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"Минимальная длительность для деактивации (мс)\""
	",\"id\":\"input1_min_inactive_time\",\"valuetype\":\"numeric\",\"value\":\"input1_min_inactive_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"SMS сообщение при активации\",\"id\":\"input1_sms_on_active\""
	",\"valuetype\":\"string\",\"value\":\"input1_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"input1_sms_on_inactive\",\"value\":\"input1_sms_on_inactive_value\",\"valuetype\":\"string\",\"minlen\":0,\"maxlen\":15"
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"input1_type\"},\"operand2\":0},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input1_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input1_type\""
	"},\"operand2\":0}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input1_type\"},\"operand2\":4}}},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input1_min_active_time\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_sms_on_active\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input1_type\""
	"},\"operand2\":3}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input1_type\"},\"operand2\":10}}},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input1_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input1_type\"},\"operand2\":9},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input1_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input1_sms_on_inactive\""
	"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"comment\":\"// INPUT 2                                                    \""
	"},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"type\":\"group\",\"label\":\"Дискретный вход 2\""
	",\"id\":\"input2\",\"collapsible\":true,\"expanded\":\"input2_expanded\",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\""
	",\"id\":\"input2_type\",\"value\":\"input2_type_value\",\"size\":40,\"items\":[{\"id\":0,\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Зажигание\""
	"},{\"id\":3,\"label\":\"Дискретный сигнал\"},{\"id\":4,\"label\":\"Импульсный сигнал\""
	"},{\"id\":5,\"label\":\"Частотный сигнал\"},{\"id\":11,\"label\":\"Кнопка вызова диспетчера\""
	"}]},{\"type\":\"text\",\"label\":\"Минимальная длительность для активации (мс)\""
	",\"id\":\"input2_min_active_time\",\"valuetype\":\"numeric\",\"value\":\"input2_min_active_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"Минимальная длительность для деактивации (мс)\""
	",\"id\":\"input2_min_inactive_time\",\"valuetype\":\"numeric\",\"value\":\"input2_min_inactive_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"SMS сообщение при активации\",\"id\":\"input2_sms_on_active\""
	",\"valuetype\":\"string\",\"value\":\"input2_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"input2_sms_on_inactive\",\"valuetype\":\"string\",\"value\":\"input2_sms_on_inactive_value\",\"minlen\":0,\"maxlen\":15"
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"input2_type\"},\"operand2\":0},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input2_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input2_type\""
	"},\"operand2\":0}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input2_type\"},\"operand2\":4}}},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input2_min_active_time\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_sms_on_active\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input2_type\""
	"},\"operand2\":3}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input2_type\"},\"operand2\":10}}},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input2_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input2_type\"},\"operand2\":9},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input2_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input2_sms_on_inactive\""
	"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"comment\":\"// INPUT 3                                                    \""
	"},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"type\":\"group\",\"label\":\"Дискретный вход 3\""
	",\"id\":\"input3\",\"collapsible\":true,\"expanded\":\"input3_expanded\",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\""
	",\"id\":\"input3_type\",\"value\":\"input3_type_value\",\"size\":40,\"items\":[{\"id\":0,\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Зажигание\""
	"},{\"id\":3,\"label\":\"Дискретный сигнал\"},{\"id\":4,\"label\":\"Импульсный сигнал\""
	"},{\"id\":5,\"label\":\"Частотный сигнал\"},{\"id\":11,\"label\":\"Кнопка вызова диспетчера\""
	"}]},{\"type\":\"text\",\"label\":\"Минимальная длительность для активации (мс)\""
	",\"id\":\"input3_min_active_time\",\"value\":\"input3_min_active_time_value\",\"valuetype\":\"numeric\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"Минимальная длительность для деактивации (мс)\""
	",\"id\":\"input3_min_inactive_time\",\"value\":\"input3_min_inactive_time_value\",\"valuetype\":\"numeric\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"SMS сообщение при активации\",\"id\":\"input3_sms_on_active\""
	",\"valuetype\":\"string\",\"value\":\"input3_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"input3_sms_on_inactive\",\"valuetype\":\"string\",\"value\":\"input3_sms_on_inactive_value\",\"minlen\":0,\"maxlen\":15"
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"input3_type\"},\"operand2\":0},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input3_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input3_type\""
	"},\"operand2\":0}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input3_type\"},\"operand2\":4}}},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input3_min_active_time\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input3_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input3_sms_on_active\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input3_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input3_type\""
	"},\"operand2\":3}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input3_type\"},\"operand2\":10}}},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input3_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input3_type\"},\"operand2\":9},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input3_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input3_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input3_sms_on_inactive\""
	"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"comment\":\"// INPUT 4                                                    \""
	"},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"type\":\"group\",\"label\":\"Дискретный вход 4\""
	",\"id\":\"input4\",\"collapsible\":true,\"expanded\":\"input4_expanded\",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\""
	",\"id\":\"input4_type\",\"size\":40,\"value\":\"input4_type_value\",\"items\":[{\"id\":0,\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Зажигание\""
	"},{\"id\":3,\"label\":\"Дискретный сигнал\"},{\"id\":4,\"label\":\"Импульсный сигнал\""
	"},{\"id\":5,\"label\":\"Частотный сигнал\"},{\"id\":7,\"label\":\"Форсунка один импульс ЭДС\""
	"},{\"id\":8,\"label\":\"Форсунка два импульса ЭДС\"},{\"id\":9,\"label\":\"Форсунка без ЭДС\""
	"},{\"id\":11,\"label\":\"Кнопка вызова диспетчера\"},{\"id\":12,\"label\":\"RS-232 RX\"}]},{\"type\":\"text\""
	",\"label\":\"Минимальная длительность для активации (мс)\""
	",\"id\":\"input4_min_active_time\",\"valuetype\":\"numeric\",\"value\":\"input4_min_active_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"Минимальная длительность для деактивации (мс)\""
	",\"id\":\"input4_min_inactive_time\",\"value\":\"input4_min_inactive_time_value\",\"valuetype\":\"numeric\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"SMS сообщение при активации\",\"id\":\"input4_sms_on_active\""
	",\"valuetype\":\"string\",\"value\":\"input4_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"input4_sms_on_inactive\",\"valuetype\":\"string\",\"value\":\"input4_sms_on_inactive_value\",\"minlen\":0,\"maxlen\":15"
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"input4_type\"},\"operand2\":0},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input4_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"input4_type\"},\"operand2\":12},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input4_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input4_type\""
	"},\"operand2\":0}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input4_type\"},\"operand2\":4}}},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input4_min_active_time\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input4_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input4_sms_on_active\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input4_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"and\",\"operand1\":{\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input4_type\""
	"},\"operand2\":3}},\"operand2\":{\"operation\":{\"operator\":\"less\",\"operand1\":{\"item\":\"input4_type\"},\"operand2\":10}}},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"input4_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"input4_type\"},\"operand2\":9},\"actions\":[{\"selector\":true"
	",\"command\":\"show\",\"item\":\"input4_min_active_time\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input4_sms_on_active\"},{\"selector\":true,\"command\":\"hide\",\"item\":\"input4_sms_on_inactive\""
	"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"comment\":\"// ADC1                                                       \""
	"},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"type\":\"group\",\"label\":\"Аналоговый вход №1\""
	",\"id\":\"adc1\",\"collapsible\":true,\"expanded\":\"adc1_expanded\",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\""
	",\"id\":\"adc1_type\",\"value\":\"adc1_type_value\",\"size\":40,\"items\":[{\"id\":0,\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Зажигание\""
	"},{\"id\":3,\"label\":\"Аналоговый сигнал\"},{\"id\":4,\"label\":\"Дискретный сигнал\""
	"}]},{\"type\":\"text\",\"label\":\"Порог срабатывания\",\"id\":\"adc1_threshold\",\"valuetype\":\"numeric\",\"value\":\"adc1_threshold_value\""
	",\"minvalue\":10}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"or\",\"operand1\":{\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"adc1_type\""
	"},\"operand2\":0}},\"operand2\":{\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"adc1_type\"},\"operand2\":3}}},\"actions\":[{\"selector\":true"
	",\"command\":\"hide\",\"item\":\"adc1_threshold\"},{\"selector\":false,\"command\":\"show\",\"item\":\"adc1_threshold\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// ADC2                                                       \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Аналоговый вход №2\",\"id\":\"adc2\",\"collapsible\":true,\"expanded\":\"adc2_expanded\""
	",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\",\"id\":\"adc2_type\",\"value\":\"adc2_type_value\",\"size\":40,\"items\":[{\"id\":0"
	",\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Зажигание\"},{\"id\":3,\"label\":\"Аналоговый сигнал\""
	"},{\"id\":4,\"label\":\"Дискретный сигнал\"},{\"id\":5,\"label\":\"Напряжение питания\""
	"}]},{\"type\":\"text\",\"label\":\"Порог срабатывания\",\"id\":\"adc2_threshold\",\"value\":\"adc2_threshold_value\""
	",\"valuetype\":\"numeric\",\"minvalue\":10}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"or\",\"operand1\":{\"operation\":{\"operator\":\"or\""
	",\"operand1\":{\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"adc2_type\"},\"operand2\":0}},\"operand2\":{\"operation\":{\"operator\":\"equal\""
	",\"operand1\":{\"item\":\"adc2_type\"},\"operand2\":3}}}},\"operand2\":{\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"adc2_type\""
	"},\"operand2\":5}}},\"actions\":[{\"selector\":true,\"command\":\"hide\",\"item\":\"adc2_threshold\"},{\"selector\":false,\"command\":\"show\""
	",\"item\":\"adc2_threshold\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// SMS                                                        \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"SMS команды\",\"items\":[{\"type\":\"text\",\"label\":\"Текст команды №1\",\"id\":\"sms1_text\""
	",\"valuetype\":\"string\",\"value\":\"sms1_text_value\",\"maxlen\":15},{\"type\":\"text\",\"label\":\"Скрипт для команды №1\""
	",\"id\":\"sms1_script\",\"valuetype\":\"string\",\"value\":\"sms1_script_value**************\",\"maxlen\":31},{\"type\":\"text\",\"label\":\"Текст команды №2\""
	",\"id\":\"sms2_text\",\"valuetype\":\"string\",\"value\":\"sms2_text_value\",\"maxlen\":15},{\"type\":\"text\",\"label\":\"Скрипт для команды №2\""
	",\"id\":\"sms2_script\",\"valuetype\":\"string\",\"value\":\"sms2_script_value**************\",\"maxlen\":31},{\"type\":\"text\",\"label\":\"Текст команды №3\""
	",\"id\":\"sms3_text\",\"value\":\"sms3_text_value\",\"valuetype\":\"string\",\"maxlen\":15},{\"type\":\"text\",\"label\":\"Скрипт для команды №3\""
	",\"id\":\"sms3_script\",\"value\":\"sms3_script_value**************\",\"valuetype\":\"string\",\"maxlen\":31},{\"type\":\"text\",\"label\":\"Текст команды №4\""
	",\"value\":\"sms4_text_value\",\"id\":\"sms4_text\",\"valuetype\":\"string\",\"maxlen\":15},{\"type\":\"text\",\"label\":\"Скрипт для команды №4\""
	",\"id\":\"sms4_script\",\"value\":\"sms4_script_value**************\",\"valuetype\":\"string\",\"maxlen\":31}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// PHONEBOOK                                                  \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Записная книга\",\"items\":[{\"type\":\"text\",\"label\":\"Телефон диспетчера (+7хххххххххх)\""
	",\"id\":\"phone\",\"valuetype\":\"string\",\"value\":\"phone_value********************\",\"maxlen\":31}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// AUDIO                                                      \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Аудио\",\"id\":\"audio\",\"collapsible\":true,\"expanded\":\"audio_expanded\",\"items\":[{\"type\":\"text\""
	",\"label\":\"Уровень динамика (1-100)\",\"id\":\"volume\",\"valuetype\":\"numeric\",\"value\":\"volume_value\""
	",\"minvalue\":1,\"maxvalue\":100},{\"type\":\"text\",\"label\":\"Уровень микрофона (1-15)\",\"id\":\"mic\",\"valuetype\":\"numeric\""
	",\"value\":\"mic_value\",\"minvalue\":1,\"maxvalue\":15},{\"type\":\"text\",\"label\":\"ES (0-7 [0])\",\"value\":\"es_value\",\"id\":\"es\",\"valuetype\":\"numeric\""
	",\"minvalue\":0,\"maxvalue\":7},{\"type\":\"text\",\"label\":\"SES (0-5) [0]\",\"value\":\"ses_value\",\"id\":\"ses\",\"valuetype\":\"numeric\",\"minvalue\":0"
	",\"maxvalue\":5},{\"type\":\"checkbox\",\"label\":\"Автоматический ответ\",\"id\":\"autoanswer\",\"checked\":\"autoanswer_checked\""
	"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"comment\":\"// ADDITIONAL FIELS                                           \""
	"},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"type\":\"group\",\"label\":\"Дополнительно передаваемые параметры\""
	",\"items\":[{\"type\":\"checkbox\",\"label\":\"Высота\",\"id\":\"altitude\",\"checked\":\"altitude_checked\"},{\"type\":\"checkbox\",\"label\":\"Направление движения\""
	",\"id\":\"cog\",\"checked\":\"cog_checked\"},{\"type\":\"checkbox\",\"label\":\"CAN обороты двигателя\",\"id\":\"rpm\""
	",\"checked\":\"rpm_checked\"}]}]";