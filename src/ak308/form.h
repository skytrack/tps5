static const unsigned char form[]="[{\"type\":\"group\",\"label\":\"Дополнительно\",\"items\":[{\"type\":\"select\",\"label\":\"Прошивка\",\"id\":\"fw\",\"size\":40"
	",\"value\":\"fw_value***********************\",\"items\":[{\"FIRMWARE\":0}]},{\"type\":\"select\",\"label\":\"Конфигурация порта\""
	",\"id\":\"port\",\"value\":\"port_value\",\"size\":40,\"items\":[{\"id\":0,\"label\":\"Два раздельных входа\"},{\"id\":1"
	",\"label\":\"RS-485\"}]},{\"type\":\"select\",\"label\":\"Сигнал активности\",\"id\":\"wakeup\",\"value\":\"wakeup_value\""
	",\"size\":40,\"items\":[{\"id\":0,\"label\":\"Всегда активен\"},{\"id\":1,\"label\":\"Напряжение питания\""
	"},{\"id\":2,\"label\":\"Акселерометр\"},{\"id\":3,\"label\":\"Дискретный вход 1\"},{\"id\":4,\"label\":\"Дискретный вход 2\""
	"}]},{\"type\":\"text\",\"label\":\"Минимальная длительность для активации (мс)\""
	",\"id\":\"wakeup_active_time\",\"valuetype\":\"numeric\",\"value\":\"wakeup_active_time_value\",\"minvalue\":10},{\"type\":\"text\""
	",\"label\":\"Минимальная длительность для деактивации (мс)\""
	",\"id\":\"wakeup_inactive_time\",\"valuetype\":\"numeric\",\"value\":\"wakeup_inactive_time_value\",\"minvalue\":10},{\"type\":\"text\""
	",\"label\":\"SMS сообщение при включении\",\"id\":\"wakeup_sms_on_active\",\"valuetype\":\"string\""
	",\"value\":\"wakeup_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при выключении\""
	",\"id\":\"wakeup_sms_on_inactive\",\"value\":\"wakeup_sms_on_inactive_value\",\"valuetype\":\"string\",\"minlen\":0,\"maxlen\":15"
	"}]},{\"type\":\"group\",\"label\":\"Сигнал напряжения питания\",\"id\":\"vcc_enable\",\"expanded\":\"vcc_expanded\""
	",\"collapsible\":true,\"items\":[{\"type\":\"checkbox\",\"label\":\"Использовать как дискретный вход 3\""
	",\"id\":\"vcc_as_di\",\"checked\":\"vcc_as_di_checked\"},{\"type\":\"text\",\"label\":\"Порог срабатывания\""
	",\"id\":\"vcc_threshold\",\"valuetype\":\"numeric\",\"value\":\"vcc_threshold_value\",\"minvalue\":0},{\"type\":\"text\",\"label\":\"Длительность высокого сигнала для активации входа (милисекунды)\""
	",\"id\":\"vcc_active_time\",\"valuetype\":\"numeric\",\"value\":\"vcc_active_time_value\",\"minvalue\":10},{\"type\":\"text\",\"label\":\"Длительность низкого сигнала для деактивации входа (милисекунды)\""
	",\"id\":\"vcc_inactive_time\",\"valuetype\":\"numeric\",\"value\":\"vcc_inactive_time_value\",\"minvalue\":10},{\"type\":\"text\""
	",\"label\":\"SMS сообщение при активации\",\"id\":\"vcc_sms_on_active\",\"valuetype\":\"string\""
	",\"value\":\"vcc_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"vcc_sms_on_inactive\",\"valuetype\":\"string\",\"value\":\"vcc_sms_on_inactive_value\",\"minlen\":0,\"maxlen\":15"
	"},{\"type\":\"checkbox\",\"label\":\"Передавать уровень напряжения\",\"id\":\"vcc_transmit\""
	",\"checked\":\"vcc_transmit_checked\"}]},{\"type\":\"group\",\"label\":\"Сигнал акселерометра\",\"id\":\"accell_enable\""
	",\"expanded\":\"accell_expanded\",\"collapsible\":true,\"items\":[{\"type\":\"checkbox\",\"label\":\"Использовать как дискретный вход 4\""
	",\"id\":\"accell_as_di\",\"checked\":\"accell_as_di_checked\"},{\"type\":\"select\",\"label\":\"Чувствительность\""
	",\"id\":\"accell_sense\",\"value\":\"accell_sense_value\",\"size\":40,\"items\":[{\"id\":3,\"label\":\"Высокая\"},{\"id\":6,\"label\":\"1\"},{\"id\":9"
	",\"label\":\"2\"},{\"id\":12,\"label\":\"3\"},{\"id\":15,\"label\":\"4\"},{\"id\":18,\"label\":\"5\"},{\"id\":21,\"label\":\"6\"},{\"id\":24,\"label\":\"7\"},{\"id\":27,\"label\":\"8\"},{\"id\":30,\"label\":\"9\"},{\"id\":33,\"label\":\"10\""
	"},{\"id\":36,\"label\":\"11\"},{\"id\":39,\"label\":\"12\"},{\"id\":42,\"label\":\"13\"},{\"id\":45,\"label\":\"14\"},{\"id\":48,\"label\":\"15\"},{\"id\":51,\"label\":\"Низкая\"}]},{\"type\":\"text\""
	",\"label\":\"Длительность активного сигнала для активации входа (сек)\""
	",\"id\":\"accell_active_time\",\"valuetype\":\"numeric\",\"value\":\"accell_active_time_value\",\"minvalue\":1,\"maxvalue\":3600"
	"},{\"type\":\"text\",\"label\":\"Длительность неактивного сигнала для деактивации входа (сек)\""
	",\"id\":\"accell_inactive_time\",\"valuetype\":\"numeric\",\"value\":\"accell_inactive_time_value\",\"minvalue\":1,\"maxvalue\":3600"
	"},{\"type\":\"text\",\"label\":\"SMS сообщение при активации\",\"id\":\"accell_sms_on_active\""
	",\"valuetype\":\"string\",\"value\":\"accell_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"accell_sms_on_inactive\",\"valuetype\":\"string\",\"value\":\"accell_sms_on_inactive_value\",\"minlen\":0,\"maxlen\":15"
	"},{\"type\":\"checkbox\",\"label\":\"Передавать сведения об ускорении\",\"id\":\"accell_transmit\""
	",\"checked\":\"accell_transmit_checked\"}]},{\"type\":\"group\",\"label\":\"Критерий стабильности уровня топлива\""
	",\"items\":[{\"type\":\"text\",\"label\":\"Предельный порог колебаний уровня топлива (в единицах датчиков)\""
	",\"id\":\"fuel_threshold\",\"valuetype\":\"numeric\",\"value\":\"fuel_threshold_value\",\"minvalue\":0},{\"type\":\"text\",\"label\":\"Период с уровнем колебаний ниже порога для фиксации стабильности уровня (сек)\""
	",\"id\":\"fuel_period\",\"valuetype\":\"numeric\",\"value\":\"fuel_period_value\",\"minvalue\":1}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\""
	",\"operand1\":{\"item\":\"port\"},\"operand2\":0},\"actions\":[{\"selector\":true,\"command\":\"show\",\"item\":\"input1\"},{\"selector\":true,\"command\":\"show\""
	",\"item\":\"input2\"},{\"selector\":true,\"command\":\"enable_item_value\",\"item\":\"wakeup\",\"value\":3},{\"selector\":true,\"command\":\"enable_item_value\""
	",\"item\":\"wakeup\",\"value\":4},{\"selector\":false,\"command\":\"disable_item_value\",\"item\":\"wakeup\",\"value\":3},{\"selector\":false"
	",\"command\":\"disable_item_value\",\"item\":\"wakeup\",\"value\":4}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"wakeup\""
	"},\"operand2\":3},\"actions\":[{\"selector\":true,\"command\":\"hide\",\"item\":\"input1\"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\""
	",\"operand1\":{\"item\":\"wakeup\"},\"operand2\":4},\"actions\":[{\"selector\":true,\"command\":\"hide\",\"item\":\"input2\"}]},{\"type\":\"condition\""
	",\"operation\":{\"operator\":\"greater\",\"operand1\":{\"item\":\"wakeup\"},\"operand2\":2},\"actions\":[{\"selector\":true,\"command\":\"show\""
	",\"item\":\"wakeup_active_time\"},{\"selector\":true,\"command\":\"show\",\"item\":\"wakeup_inactive_time\"},{\"selector\":true"
	",\"command\":\"show\",\"item\":\"wakeup_sms_on_active\"},{\"selector\":true,\"command\":\"show\",\"item\":\"wakeup_sms_on_inactive\""
	"},{\"selector\":false,\"command\":\"hide\",\"item\":\"wakeup_active_time\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"wakeup_inactive_time\""
	"},{\"selector\":false,\"command\":\"hide\",\"item\":\"wakeup_sms_on_active\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"wakeup_sms_on_inactive\""
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"port\"},\"operand2\":0},\"actions\":[{\"selector\":false,\"command\":\"hide\""
	",\"item\":\"input1\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"input2\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// ACTIVE MODE                                                \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Активный режим\",\"items\":[{\"type\":\"text\",\"label\":\"Интервал замера координат (сек)\""
	",\"id\":\"active_interval\",\"valuetype\":\"numeric\",\"value\":\"active_interval_value\",\"minvalue\":1,\"maxvalue\":3600},{\"type\":\"text\""
	",\"label\":\"Количество точек в посылке\",\"id\":\"active_ppp\",\"minvalue\":1,\"maxvalue\":4"
	",\"value\":\"active_ppp_value\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// WAIT MODE                                                  \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Ждущий режим\",\"id\":\"wait_enable\",\"collapsible\":true,\"expanded\":\"wait_expanded\""
	",\"items\":[{\"type\":\"text\",\"label\":\"Задержка перехода в ждущий режим после прекращения активности (минуты)\""
	",\"id\":\"wait_delay\",\"valuetype\":\"numeric\",\"value\":\"wait_delay_value\",\"minvalue\":0,\"maxvalue\":9999},{\"type\":\"text\",\"label\":\"Интервал замера координат (минуты) [1..1440]\""
	",\"id\":\"wait_interval\",\"valuetype\":\"numeric\",\"value\":\"wait_interval_value\",\"minvalue\":1,\"maxvalue\":1440},{\"type\":\"checkbox\""
	",\"label\":\"Переходить в режим только при стабильном уровне топлива\""
	",\"id\":\"wait_after_fuel\",\"checked\":\"wait_after_fuel_checked\"},{\"type\":\"checkbox\",\"label\":\"Переходить в активный режим при изменении уровня топлива\""
	",\"id\":\"wait_to_active_fuel\",\"checked\":\"wait_to_active_fuel_checked\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// SLEEP MODE                                                 \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Спящий режим\",\"id\":\"sleep_enable\",\"collapsible\":true,\"expanded\":\"sleep_expanded\""
	",\"items\":[{\"type\":\"text\",\"label\":\"Задержка перехода в спящий режим после прекращения активности (минуты)\""
	",\"id\":\"sleep_delay\",\"valuetype\":\"numeric\",\"value\":\"sleep_delay_value\",\"minvalue\":0,\"maxvalue\":9999},{\"type\":\"text\",\"label\":\"Интервал замера координат (часы) [1..24]\""
	",\"id\":\"sleep_interval\",\"valuetype\":\"numeric\",\"value\":\"sleep_interval_value\",\"minvalue\":1,\"maxvalue\":24},{\"type\":\"text\""
	",\"label\":\"Максимальное время поиска GPS сигнала (минуты)\""
	",\"id\":\"sleep_max_nav_search\",\"valuetype\":\"numeric\",\"value\":\"sleep_nav_value\",\"minvalue\":0,\"maxvalue\":9999},{\"type\":\"text\""
	",\"label\":\"Максимальное время поиска GSM сигнала (минуты)\""
	",\"id\":\"sleep_max_gsm_search\",\"valuetype\":\"numeric\",\"value\":\"sleep_gsm_value\",\"minvalue\":0,\"maxvalue\":9999},{\"type\":\"checkbox\""
	",\"label\":\"Переходить в режим только при стабильном уровне топлива\""
	",\"id\":\"sleep_after_fuel\",\"checked\":\"sleep_after_fuel_checked\"},{\"type\":\"checkbox\",\"label\":\"Переходить в активный режим при изменении уровня топлива\""
	",\"id\":\"sleep_to_active_fuel\",\"checked\":\"sleep_to_active_fuel_checked\"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// INPUT 1                                                    \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Дискретный вход 1\",\"id\":\"input1\",\"collapsible\":true,\"expanded\":\"input1_expanded\""
	",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\",\"id\":\"input1_type\",\"value\":\"input1_type_value\",\"size\":40"
	",\"items\":[{\"id\":0,\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Дискретный датчик\"},{\"id\":2,\"label\":\"Импульсный датчик\""
	"},{\"id\":3,\"label\":\"Частотный датчик\"},{\"id\":4,\"label\":\"Частотный ДУТ\"},{\"id\":5,\"label\":\"RS-485, сигнал B, (левый дут)\""
	"}]},{\"type\":\"text\",\"label\":\"Минимальная длительность для активации (мс)\""
	",\"id\":\"input1_min_active_time\",\"valuetype\":\"numeric\",\"value\":\"input1_min_active_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"Минимальная длительность для деактивации (мс)\""
	",\"id\":\"input1_min_inactive_time\",\"valuetype\":\"numeric\",\"value\":\"input1_min_inactive_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"SMS сообщение при активации\",\"id\":\"input1_sms_on_active\""
	",\"valuetype\":\"string\",\"value\":\"input1_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"input1_sms_on_inactive\",\"value\":\"input1_sms_on_inactive_value\",\"valuetype\":\"string\",\"minlen\":0,\"maxlen\":15"
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"input1_type\"},\"operand2\":1},\"actions\":[{\"selector\":false"
	",\"command\":\"hide\",\"item\":\"input1_min_active_time\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"input1_min_inactive_time\""
	"},{\"selector\":false,\"command\":\"hide\",\"item\":\"input1_sms_on_active\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"input1_sms_on_inactive\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_min_active_time\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_sms_on_active\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input1_sms_on_inactive\""
	"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"comment\":\"// INPUT 2                                                    \""
	"},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"type\":\"group\",\"label\":\"Дискретный вход 2\""
	",\"id\":\"input2\",\"collapsible\":true,\"expanded\":\"input2_expanded\",\"items\":[{\"type\":\"select\",\"label\":\"Подключение\""
	",\"id\":\"input2_type\",\"value\":\"input2_type_value\",\"size\":40,\"items\":[{\"id\":0,\"label\":\"Не подключен\"},{\"id\":1,\"label\":\"Дискретный датчик\""
	"},{\"id\":2,\"label\":\"Импульсный датчик\"},{\"id\":3,\"label\":\"Частотный датчик\"},{\"id\":4"
	",\"label\":\"Частотный ДУТ\"},{\"id\":5,\"label\":\"RS-485, сигнал B, (правый дут)\""
	"}]},{\"type\":\"text\",\"label\":\"Минимальная длительность для активации (мс)\""
	",\"id\":\"input2_min_active_time\",\"valuetype\":\"numeric\",\"value\":\"input2_min_active_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"Минимальная длительность для деактивации (мс)\""
	",\"id\":\"input2_min_inactive_time\",\"valuetype\":\"numeric\",\"value\":\"input2_min_inactive_time_value\",\"minvalue\":10"
	"},{\"type\":\"text\",\"label\":\"SMS сообщение при активации\",\"id\":\"input2_sms_on_active\""
	",\"valuetype\":\"string\",\"value\":\"input2_sms_on_active_value\",\"minlen\":0,\"maxlen\":15},{\"type\":\"text\",\"label\":\"SMS сообщение при деактивации\""
	",\"id\":\"input2_sms_on_inactive\",\"valuetype\":\"string\",\"value\":\"input2_sms_on_inactive_value\",\"minlen\":0,\"maxlen\":15"
	"}]},{\"type\":\"condition\",\"operation\":{\"operator\":\"equal\",\"operand1\":{\"item\":\"input2_type\"},\"operand2\":1},\"actions\":[{\"selector\":false"
	",\"command\":\"hide\",\"item\":\"input2_min_active_time\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"input2_min_inactive_time\""
	"},{\"selector\":false,\"command\":\"hide\",\"item\":\"input2_sms_on_active\"},{\"selector\":false,\"command\":\"hide\",\"item\":\"input2_sms_on_inactive\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_min_active_time\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_min_inactive_time\""
	"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_sms_on_active\"},{\"selector\":true,\"command\":\"show\",\"item\":\"input2_sms_on_inactive\""
	"}]},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"comment\":\"// PHONEBOOK                                                  \""
	"},{\"comment\":\"//////////////////////////////////////////////////////////////\"},{\"type\":\"group\",\"label\":\"Записная книга\""
	",\"items\":[{\"type\":\"text\",\"label\":\"Телефон диспетчера (+7хххххххххх)\",\"id\":\"phone\""
	",\"valuetype\":\"string\",\"value\":\"phone_value********************\",\"maxlen\":31}]},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"comment\":\"// ADDITIONAL FIELDS                                          \"},{\"comment\":\"//////////////////////////////////////////////////////////////\""
	"},{\"type\":\"group\",\"label\":\"Дополнительно передаваемые параметры\""
	",\"items\":[{\"type\":\"checkbox\",\"label\":\"Высота\",\"id\":\"altitude\",\"checked\":\"altitude_checked\"},{\"type\":\"checkbox\",\"label\":\"Направление движения\""
	",\"id\":\"cog\",\"checked\":\"cog_checked\"}]}]";