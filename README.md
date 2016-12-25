msu3-waves
==========

Fourth task for the CG course, GPU-driven water simulation.
The vast majority of ideas have been borrowed from http://madebyevan.com/
In fact, this is just a rough port of his «WebGL Water» into C.
Compatible with Linux, Win32 and WINE.

(original description in Russian:)

Задание:               4 / OpenGL-симуляция воды

Базовая часть:
  Наличие пола и стен [+]
  Объекты под водой   [+]
  Прозрачность воды   [+]
  Реалистичный свет   [+]
  Интерактивность     [+]
  FPS-независ. обсчёт [+]

Дополнительная часть:
  Текстурирование     [+]
  Отражение           [+]
  Преломление         [+]
  Каустика            [+]
  Управление камерой  [+]
  Предустановки камер [+]
  Многопоточность     [+]

Система:               MinGW / Code::Blocks 10.05
ОС:                    Windows XP SP2, Windows 7 Home Extended, WINE 1.5.12-1
Аппаратура:            Core2Duo T5500 1.66 ГГц, 2048 МБ, Radeon X1700 Mobility

Комментарии:
Использовались plain C, библиотеки Win32 API (для Windows-версии) и библиотеки
GTK2 и GTKGLext (для Linux-версии). Необходим OpenGL v2.0 или выше.
Вращать камеру можно, зажав левую кнопку мыши, а "бегать" - нажимая W/A/S/D.
Вся геометрия и текстуры генерируются процедурно.

Дополнительный интерактив:
  - пробел/ПКМ: сбросить на воду каплю
  - клавиша F1: включить/выключить каркасное отображение
  - клавиша F2: сменить цветовую палитру
  - клавиша F4: убрать волнение с поверхности
  - клавиша F5: приостановить / возобновить анимацию
