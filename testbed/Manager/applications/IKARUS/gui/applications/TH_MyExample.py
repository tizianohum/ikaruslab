import math
import random
import threading
import time

import numpy as np

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

    arming_button = Button(widget_id='arming_btn', text='not armed')
    page.addWidget(arming_button, width=5, height=5, row=1, column=1)

    def toggle_arming(*a, **k):
        armed_state["armed"] = not armed_state["armed"]
        if armed_state["armed"]:
            arming_button.updateConfig(text='ARMED', color=[1, 0, 0, 1])
            print("System armed!")
        else:
            arming_button.updateConfig(text='not armed', color=[0.8, 0.8, 0.8, 1])
            print("System disarmed.")

    arming_button.callbacks.click.register(toggle_arming)
    # Sliders
    slider1 = SliderWidget(widget_id='slider1', min_value=0,
                           max_value=1,
                           increment=0.1,
                           value=0.5,
                           color=random_color_from_palette('dark'),
                           continuousUpdates=True,
                           automaticReset=0.5)
    page.addWidget(slider1, height=2, width=5)
    slider1_2 = SliderWidget(widget_id='slider1u2', min_value=0,
                           max_value=1,
                           increment=0.1,
                           value=0.5,
                           color=random_color_from_palette('dark'),
                           continuousUpdates=True,
                           automaticReset=0.5)
    page.addWidget(slider1_2, height=2, width=5, row=3)

    slider2 = SliderWidget(widget_id='slider2',
                           min_value=0,
                           max_value=100,
                           increment=0.1,
                           value=20,
                           color=random_color_from_palette('dark'),
                           direction='vertical',
                           ticks=[0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100], )

    page.addWidget(slider2, height=6, width=2, column=1, row=8)
    # Plots

    pw1 = RT_Plot_Widget(widget_id="pw1",
                         title='Plot 11',
                         use_local_time=True,
                         x_axis_config={
                             'window_time': 15
                         }
                         )

    y_axis_1 = Y_Axis(id="y_axis_1",
                      label='Axis 1',
                      min=-10,
                      max=10,
                      grid=True
                      )

    pw1.plot.add_y_axis(y_axis_1)
    ds1 = TimeSeries(id="ds1",
                     y_axis="y_axis_1",
                     name='Data 1',
                     color=random_color_from_palette('pastel'))

    pw1.plot.add_timeseries(ds1)

    ds2 = TimeSeries(id="ds2",
                     y_axis="y_axis_1",
                     name='Data 2',
                     color=random_color_from_palette('pastel'))

    pw1.plot.add_timeseries(ds2)

    def test_remove_timeseries():
        pw1.plot.remove_timeseries(ds2)

    setTimeout(test_remove_timeseries, 4)

    page.addWidget(pw1, width=20, height=10, row=1, column=31)



    # GUI starten
    app.start()
    # Endlosschleife, damit das Programm nicht beendet wird
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("GUI beendet.")


if __name__ == '__main__':
    main()