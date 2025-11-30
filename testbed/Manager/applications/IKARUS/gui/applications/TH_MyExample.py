import math
import random
import threading
import time

import numpy as np

from applications.IKARUS.gui.applications.communication import MOTOR1_BEEP
from core.utils.colors import random_color, random_color_from_palette
from core.utils.plotting import run_periodic, safe_close, new_figure_agg, quick_line_plot_data_uri, fig_to_data_uri, \
    save_figure
from core.utils.time import delayed_execution, setInterval, setTimeout
from extensions.gui.src.gui import GUI, Category, Page
from extensions.gui.src.lib.map.map import MapWidget
from extensions.gui.src.lib.map.map_objects import Point, Agent, VisionAgent, CoordinateSystem, MapObjectGroup, Line
from extensions.gui.src.lib.objects.objects import Widget_Group, ContextMenuItem, ContextMenuGroup, \
    PagedWidgetGroup, \
    GroupPageWidget, ContainerWrapper, GUI_Container_Stack, GUI_Container, GUI_CollapsibleContainer
from extensions.gui.src.lib.objects.python.bilbo import BILBO_Widget
from extensions.gui.src.lib.objects.python.callout import Callout, CalloutType
from extensions.gui.src.lib.objects.python.checkbox import CheckboxWidget
from extensions.gui.src.lib.objects.python.directory import DirectoryWidget
from extensions.gui.src.lib.objects.python.indicators import CircleIndicator, LoadingIndicator, ProgressIndicator
from extensions.gui.src.lib.objects.python.popup import Popup
from extensions.gui.src.lib.plot.lineplot.lineplot_widget import LinePlotWidget
from extensions.gui.src.lib.plot.realtime.rt_plot import RT_Plot_Widget, TimeSeries, Y_Axis
from extensions.gui.src.lib.objects.python.buttons import Button, MultiStateButton
from extensions.gui.src.lib.objects.python.image import ImageWidget, UpdatableImageWidget
from extensions.gui.src.lib.objects.python.number import DigitalNumberWidget
from extensions.gui.src.lib.objects.python.sliders import SliderWidget, ClassicSliderWidget
from extensions.gui.src.lib.objects.python.select import MultiSelectWidget
from extensions.gui.src.lib.objects.python.dial import RotaryDialWidget
from extensions.gui.src.lib.objects.python.table import TableWidget
from extensions.gui.src.lib.objects.python.text import TextWidget, StatusWidget, StatusWidgetElement
from extensions.gui.src.lib.objects.python.text_input import InputWidget
from core.utils.network.network import getHostIP
from extensions.gui.src.lib.objects.python.video import VideoWidget
from extensions.gui.src.lib.terminal.terminal_widget import TerminalWidget
import random

import communication

com = communication.Communication()

def main():
    # IP-Adresse holen (Server-Adresse)
    host = getHostIP()

    # Haupt-GUI erstellen
    app = GUI(id='gui', host=host, run_js=True)

    # Kategorie erstellen
    category = Category(id='main_cat', name='Main')

    # Seite erstellen
    page = Page(id='main_page', name='Control Desk')

    # Kategorie zur App hinzufügen
    app.addCategory(category)
    category.addPage(page)

    # --- Arming Toggle Button ---
    armed_state = {"armed": False}

    arming_button = Button(widget_id='arming_btn', text='not armed', color=[0.5, 0.5, 0, 1])
    page.addWidget(arming_button, width=5, height=5, row=1, column=23)

    def toggle_arming(*a, **k):
        armed_state["armed"] = not armed_state["armed"]
        if armed_state["armed"]:
            arming_button.updateConfig(text='System is ARMED', color=[1, 0, 0, 1])
            com.send_arming(True)
            print("System armed!")
        else:
            arming_button.updateConfig(text='System not armed', color=[0.5, 0.5, 0, 1])
            com.send_arming(False)
            print("System disarmed.")

    arming_button.callbacks.click.register(toggle_arming)

    mag_calibrate_button = Button(widget_id='mag_cal_btn', text='Calibrate Magnetometer', color=[0, 0.5, 0.5, 1])
    page.addWidget(mag_calibrate_button, width=5, height=2, row=7, column=23)
    def calibrate_magnetometer(*a, **k):
        com.send_mag_calibration()
        print("Magnetometer calibration command sent.")

    mag_calibrate_button.callbacks.click.register(calibrate_magnetometer)

    motor_beep_buttons = []
    for i in range(1, 5):
        motor_beep_button = Button(
            widget_id=f'motor{i}_beep_btn',
            text=f'Motor {i} Beep',
            color=[0.5, 0, 0.5, 1]
        )
        page.addWidget(motor_beep_button, width=5, height=2, row=8 + i * 2, column=23)

        def motor_beep(*a, motor_id=i, **k):
            com.send_special_command(motor_id)
            print(f"Motor {motor_id} Beep command sent.")

        motor_beep_button.callbacks.click.register(motor_beep)
        motor_beep_buttons.append(motor_beep_button)

    # Buttons für "Reverse Spin"-Befehl
    motor_reverse_spin_buttons = []
    motor_reverse_spin_commands = [5, 6, 7, 8]  # Command IDs für Reverse Spin
    for i, command_id in enumerate(motor_reverse_spin_commands, start=1):
        motor_reverse_spin_button = Button(
            widget_id=f'motor{i}_reverse_spin_btn',
            text=f'Motor {i} Reverse Spin',
            color=[0, 0.5, 0.5, 1]
        )
        page.addWidget(motor_reverse_spin_button, width=5, height=2, row=8 + i * 2, column=30)

        def motor_reverse_spin(*a, cmd_id=command_id, **k):
            com.send_special_command(cmd_id)
            print(f"Motor {i} Reverse Spin command sent.")

        motor_reverse_spin_button.callbacks.click.register(motor_reverse_spin)
        motor_reverse_spin_buttons.append(motor_reverse_spin_button)

    # Sliders
    slider1 = SliderWidget(widget_id='slider1', min_value=0,
                           max_value=1,
                           increment=0.1,
                           value=0.5,
                           color=random_color_from_palette('bright'),
                           continuousUpdates=True,
                           automaticReset=0.5)
    page.addWidget(slider1, height=2, width=5)

    def on_slider1_change(value, *a, **k):
        print(f"Slider 1 value changed to: {value}")

    slider1.callbacks.value_changed.register(on_slider1_change)
    slider1_2 = SliderWidget(widget_id='slider1u2', min_value=0,
                           max_value=1,
                           increment=0.1,
                           value=0.5,
                           color=random_color_from_palette('bright'),
                           continuousUpdates=True,
                           automaticReset=0.5)
    page.addWidget(slider1_2, height=2, width=5, row=3)

    slider_roll = SliderWidget(widget_id='roll',
                           min_value=-45,
                           max_value=45,
                           increment=0.1,
                           value=0,
                           color=[0, 0, 1, 1],
                           direction='vertical',
                           ticks=list(range(-45, 46, 5)), )

    page.addWidget(slider_roll, height=6, width=2, column=1, row=8)
    def on_roll_change(value, *a, **k):
        com.send_roll(value)
        print(f"roll value changed to: {value}")

    slider_roll.callbacks.value_changed.register(on_roll_change)

    slider_pitch = SliderWidget(widget_id='pitch',
                           min_value=-45,
                           max_value=45,
                           increment=0.1,
                           value=0,
                           color=[0.5, 0, 0.5, 1],
                           direction='vertical',
                           ticks=list(range(-45, 46, 5)), )

    page.addWidget(slider_pitch, height=6, width=2, column=3, row=8)
    def on_pitch_change(value, *a, **k):
        com.send_pitch(value)
        print(f"pitch value changed to: {value}")

    slider_pitch.callbacks.value_changed.register(on_pitch_change)

    slider_yaw = SliderWidget(widget_id='yaw',
                           min_value=-180,
                           max_value=180,
                           increment=0.1,
                           value=0,
                           color=[0, 1, 0, 1],
                           direction='vertical',
                           ticks=list(range(-180, 181, 45)), )

    page.addWidget(slider_yaw, height=6, width=2, column=5, row=8)
    def on_yaw_change(value, *a, **k):
        com.send_yaw(value)
        print(f"yaw value changed to: {value}")

    slider_yaw.callbacks.value_changed.register(on_yaw_change)


    ### Motor Thrust Control ###
    # Slider für Motoren
    motor_colors = [
        [1, 0, 0, 1],  # Rot
        [0, 1, 0, 1],  # Grün
        [0, 0, 1, 1],  # Blau
        [1, 1, 0, 1]  # Gelb
    ]

    motor_sliders = []
    for i, color in enumerate(motor_colors):
        motor_slider = SliderWidget(
            widget_id=f'motor_{i + 1}',
            min_value=0,
            max_value=2000,
            increment=10,
            value=0,
            color=color,
            direction='vertical',
            ticks=list(range(0, 2000, 100)),
        )
        page.addWidget(motor_slider, height=6, width=2, column=10 + i * 2, row=8)

        def on_motor_change(value, motor_id=i + 1, *a, **k):
            if motor_id == 1:
                com.send_motor1(value)
            elif motor_id == 2:
                com.send_motor2(value)
            elif motor_id == 3:
                com.send_motor3(value)
            elif motor_id == 4:
                com.send_motor4(value)
            print(f"Motor {motor_id} thrust changed to: {value}")

        motor_slider.callbacks.value_changed.register(on_motor_change)
        motor_sliders.append(motor_slider)

    # Plots

    pw1 = RT_Plot_Widget(
        widget_id="pw1",
        title='Roll / Pitch / Yaw',
        use_local_time=True,
        x_axis_config={
            'window_time': 15
        }
    )

    # Eine gemeinsame Y-Achse für alle drei Werte
    y_axis_1 = Y_Axis(
        id="y_axis_1",
        label='Angle [°]',
        min=-15,
        max=15,
        grid=True
    )

    pw1.plot.add_y_axis(y_axis_1)

    # --- TimeSeries ---
    ds_roll = TimeSeries(
        id="roll",
        y_axis="y_axis_1",
        name='Roll',
        color=random_color_from_palette('pastel')
    )
    pw1.plot.add_timeseries(ds_roll)

    ds_pitch = TimeSeries(
        id="pitch",
        y_axis="y_axis_1",
        name='Pitch',
        color=random_color_from_palette('pastel')
    )
    pw1.plot.add_timeseries(ds_pitch)

    ds_yaw = TimeSeries(
        id="yaw",
        y_axis="y_axis_1",
        name='Yaw',
        color=random_color_from_palette('pastel')
    )
    pw1.plot.add_timeseries(ds_yaw)

    # Plot für Ultraschall-Daten
    pw2 = RT_Plot_Widget(
        widget_id="pw2",
        title='Ultrasonic Data',
        use_local_time=True,
        x_axis_config={
            'window_time': 15
        }
    )

    # Y-Achse für Ultraschall-Daten
    y_axis_ultrasonic = Y_Axis(
        id="y_axis_ultrasonic",
        label='Distance [m]',
        min=0,
        max=100,
        grid=True
    )

    pw2.plot.add_y_axis(y_axis_ultrasonic)

    # TimeSeries für Ultraschall-Daten
    ds_ultrasonic = TimeSeries(
        id="ultrasonic",
        y_axis="y_axis_ultrasonic",
        name='Ultrasonic',
        color=random_color_from_palette('pastel')
    )
    pw2.plot.add_timeseries(ds_ultrasonic)

    # Plot-Widget zur Seite hinzufügen (über dem aktuellen Plot)
    page.addWidget(pw2, width=15, height=8, row=3, column=36)


    # def test_remove_timeseries():
    #     pw1.plot.remove_timeseries(ds_roll)
    #
    # setTimeout(test_remove_timeseries, 4)

    page.addWidget(pw1, width=15, height=8, row=11, column=36)



    # GUI starten
    print("Done.")
    app.start()
    # Endlosschleife, damit das Programm nicht beendet wird
    try:
        while True:
            ds_roll.set_value(com.roll)
            ds_pitch.set_value(com.pitch)
            ds_yaw.set_value(com.yaw)
            ds_ultrasonic.set_value(com.ultrasonic)
            time.sleep(1)
    except KeyboardInterrupt:
        print("GUI beendet.")


if __name__ == '__main__':
    main()