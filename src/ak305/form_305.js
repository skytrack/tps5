[
	{ 
		"type": "group", 
		"label": "Дополнительно", 
		"items": [
			{
				"type": "select",
				"label": "Прошивка",
				"id": "fw",
				"size": 40,
				"value": "fw_value***********************",
				"items": [ {"FIRMWARE":0}
				]
			}
		]
	},
	
{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// ACTIVE MODE                                                "},
{ "comment": "//////////////////////////////////////////////////////////////"},
	{ 
		"type": "group", 
		"label": "Активный режим", 
		"items": [
			{
				"type": "text",
				"label": "Интервал замера координат (сек)",
				"id": "active_interval",
				"valuetype": "numeric",
				"value": "active_interval_value",
				"minvalue": 1,
				"maxvalue": 3600
			},
			{
				"type": "text",
				"label": "Количество точек в посылке",
				"id": "active_ppp",
				"valuetype": "numeric",
				"minvalue": 1,
				"maxvalue": 4,
				"value": "active_ppp_value"
			},
			{
				"type": "checkbox",
				"label": "Отключить статическую навигацию",
				"id": "disable_static",
				"checked": "disable_static_checked"
			}
		]
	},

{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// PARK MODE                                                  "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "Ждущий режим",
		"id": "wait_enable", 
		"collapsible": false,
		"items": [
			{
				"type": "text",
				"label": "Интервал замера координат (минуты) [1..1440]",
				"id": "park_interval",
				"valuetype": "numeric",
				"value": "park_interval_value",
				"minvalue": 1,
				"maxvalue": 1440
			},
			{
				"type": "text",
				"label": "Количество точек в посылке",
				"valuetype": "numeric",
				"id": "park_ppp",
				"minvalue": 1,
				"maxvalue": 4,
				"value": "park_ppp_value"
			}
		]
	},
{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// INPUT 1                                                    "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "Дискретный вход 1", 
		"id": "input1",
		"collapsible": true,
		"expanded": "input1_expanded",
		"items": [
			{
				"type": "select",
				"label": "Подключение",
				"id": "input1_type",
				"value": "input1_type_value",
				"size": 40,
				"items": [
					{ "id": 0, "label": "Не подключен"},
					{ "id": 1, "label": "Зажигание"},
					{ "id": 3, "label": "Дискретный датчик"},
					{ "id": 4, "label": "Импульсный датчик"},
					{ "id": 10,"label": "Кнопка вызова диспетчера"},
					{ "id": 9, "label": "Форсунка без ЭДС"}
				]
			},
			{
				"type": "text",
				"label": "SMS сообщение при активации",
				"id": "input1_sms_on_active",
				"valuetype": "string",
				"value": "input1_sms_on_active_value",
				"minlen": 0,
				"maxlen": 15
			}
		]
	},
	{
		"type": "condition",
		"operation": {
			"operator": "or",
			"operand1": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input1_type" },
					"operand2": 1
				}
			},
			"operand2": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input1_type" },
					"operand2": 2
				}
			}
		},
		"actions": [
			{ "selector": true,  "command":"show", "item":"input1_sms_on_active"},
			{ "selector": false, "command":"hide", "item":"input1_sms_on_active"}
		]
	},

{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// INPUT 2                                                    "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "Дискретный вход 2", 
		"id": "input2",
		"collapsible": true,
		"expanded": "input2_expanded",
		"items": [
			{
				"type": "select",
				"label": "Подключение",
				"id": "input2_type",
				"value": "input2_type_value",
				"size": 40,
				"items": [
					{ "id": 0, "label": "Не подключен"},
					{ "id": 1, "label": "Зажигание"},
					{ "id": 3, "label": "Дискретный сигнал"},
					{ "id": 4, "label": "Импульсный сигнал"},
					{ "id": 10, "label": "Кнопка вызова диспетчера"}
				]
			},
			{
				"type": "text",
				"label": "SMS сообщение при активации",
				"id": "input2_sms_on_active",
				"valuetype": "string",
				"value": "input2_sms_on_active_value",
				"minlen": 0,
				"maxlen": 15
			}
		]
	},
	{
		"type": "condition",
		"operation": {
			"operator": "or",
			"operand1": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input2_type" },
					"operand2": 1
				}
			},
			"operand2": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input2_type" },
					"operand2": 2
				}
			}
		},
		"actions": [
			{ "selector": true, "command":"show", "item":"input2_sms_on_active"},
			{ "selector": false, "command":"hide", "item":"input2_sms_on_active"}
		]
	},

{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// INPUT 3                                                    "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "Дискретный вход 3", 
		"id": "input3",
		"collapsible": true,
		"expanded": "input3_expanded",
		"items": [
			{
				"type": "select",
				"label": "Подключение",
				"id": "input3_type",
				"value": "input3_type_value",
				"size": 40,
				"items": [
					{ "id": 0, "label": "Не подключен"},
					{ "id": 1, "label": "Зажигание"},
					{ "id": 3, "label": "Дискретный сигнал"},
					{ "id": 4, "label": "Импульсный сигнал"},
					{ "id": 10,"label": "Кнопка вызова диспетчера"}
				]
			},
			{
				"type": "text",
				"label": "SMS сообщение при активации",
				"id": "input3_sms_on_active",
				"valuetype": "string",
				"value": "input3_sms_on_active_value",
				"minlen": 0,
				"maxlen": 15
			}
		]
	},
	{
		"type": "condition",
		"operation": {
			"operator": "or",
			"operand1": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input3_type" },
					"operand2": 1
				}
			},
			"operand2": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input3_type" },
					"operand2": 2
				}
			}
		},
		"actions": [
			{ "selector": true, "command":"show", "item":"input3_sms_on_active"},
			{ "selector": false, "command":"hide", "item":"input3_sms_on_active"}
		]
	},

{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// INPUT 4                                                    "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "Дискретный вход 4", 
		"id": "input4",
		"collapsible": true,
		"expanded": "input4_expanded",
		"items": [
			{
				"type": "select",
				"label": "Подключение",
				"id": "input4_type",
				"size": 40,
				"value": "input4_type_value",
				"items": [
					{ "id": 0, "label": "Не подключен"},
					{ "id": 1, "label": "Зажигание"},
					{ "id": 3, "label": "Дискретный сигнал"},
					{ "id": 4, "label": "Импульсный сигнал"},
					{ "id": 10,"label": "Кнопка вызова диспетчера"}
				]
			},
			{
				"type": "text",
				"label": "SMS сообщение при активации",
				"id": "input4_sms_on_active",
				"valuetype": "string",
				"value": "input4_sms_on_active_value",
				"minlen": 0,
				"maxlen": 15
			}
		]
	},
	{
		"type": "condition",
		"operation": {
			"operator": "or",
			"operand1": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input4_type" },
					"operand2": 1
				}
			},
			"operand2": {
				"operation": {
					"operator": "equal",
					"operand1": { "item":"input4_type" },
					"operand2": 2
				}
			}
		},
		"actions": [
			{ "selector": true, "command":"show", "item":"input4_sms_on_active"},
			{ "selector": false, "command":"hide", "item":"input4_sms_on_active"}
		]
	},
{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// SMS                                                        "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "SMS команды", 
		"items": [
			{
				"type": "text",
				"label": "Команда активации выхода №1",
				"id": "sms_a1",
				"valuetype": "string",
				"value": "sms_a1_value***",
				"maxlen": 15
			},
			{
				"type": "text",
				"label": "Команда деактивации выхода №1",
				"id": "sms_d1",
				"valuetype": "string",
				"value": "sms_d1_value***",
				"maxlen": 15
			},
			{
				"type": "text",
				"label": "Команда активации выхода №2",
				"id": "sms_a2",
				"valuetype": "string",
				"value": "sms_a2_value***",
				"maxlen": 15
			},
			{
				"type": "text",
				"label": "Команда деактивации выхода №2",
				"id": "sms_d2",
				"valuetype": "string",
				"value": "sms_d2_value***",
				"maxlen": 15
			},
			{
				"type": "text",
				"label": "Команда активации выхода №3",
				"id": "sms_a3",
				"valuetype": "string",
				"value": "sms_a3_value***",
				"maxlen": 15
			},
			{
				"type": "text",
				"label": "Команда деактивации выхода №3",
				"id": "sms_d3",
				"valuetype": "string",
				"value": "sms_d3_value***",
				"maxlen": 15
			},
			{
				"type": "text",
				"label": "Команда активации выхода №4",
				"id": "sms_a4",
				"valuetype": "string",
				"value": "sms_a4_value***",
				"maxlen": 15
			},
			{
				"type": "text",
				"label": "Команда деактивации выхода №4",
				"id": "sms_d4",
				"valuetype": "string",
				"value": "sms_d4_value***",
				"maxlen": 15
			}
		]
	},

{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// PHONEBOOK                                                  "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "Записная книга", 
		"items": [
			{
				"type": "text",
				"label": "Телефон диспетчера (+7хххххххххх)",
				"id": "phone",
				"valuetype": "string",
				"value": "phone_value********************",
				"maxlen": 31
			}
		]
	},

{ "comment": "//////////////////////////////////////////////////////////////"},
{ "comment": "// AUDIO                                                      "},
{ "comment": "//////////////////////////////////////////////////////////////"},

	{ 
		"type": "group", 
		"label": "Аудио", 
		"id": "audio",
		"collapsible": true,
		"expanded": "audio_expanded",
		"items": [
			{
				"type": "text",
				"label": "Уровень динамика (1-100)",
				"id": "volume",
				"valuetype": "numeric",
				"value": "volume_value",
				"minvalue": 1,
				"maxvalue": 100
			},
			{
				"type": "text",
				"label": "Уровень микрофона (1-15)",
				"id": "mic",
				"valuetype": "numeric",
				"value": "mic_value",
				"minvalue": 1,
				"maxvalue": 15
			},
			{
				"type": "text",
				"label": "Модель связи (1-32767 [4000])",
				"value": "echo_model_value",
				"id": "echo_model",
				"valuetype": "numeric",
				"minvalue": 0,
				"maxvalue": 32767
			},
			{
				"type": "text",
				"label": "Степень насыщения микрофона (1-32767 [20])",
				"value": "echo_level_value",
				"id": "echo_level",
				"valuetype": "numeric",
				"minvalue": 0,
				"maxvalue": 32767
			},
			{
				"type": "text",
				"label": "Коэффициент затирания эха (1-32767 [4])",
				"value": "echo_patterns_value",
				"id": "echo_patterns",
				"valuetype": "numeric",
				"minvalue": 0,
				"maxvalue": 32767
			},
			{
				"type": "checkbox",
				"label": "Автоматический ответ",
				"id": "autoanswer",
				"checked": "autoanswer_checked"
			}
		]
	}
]