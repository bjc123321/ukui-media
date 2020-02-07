/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "deiceswitchwidget.h"
extern "C" {
#include <gdk/gdk.h>
#include <glib-object.h>
#include <glib/gi18n.h>
}
#include <QDebug>
extern "C" {
#include <glib-object.h>
#include <glib.h>
#include <gio/gio.h>
#include <gobject/gparamspecs.h>
#include <glib/gi18n.h>
}
#include <XdgIcon>
#include <XdgDesktopFile>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QStringList>
#include <QSpacerItem>
#include <QListView>
#include <QDebug>

typedef enum {
    DEVICE_VOLUME_BUTTON,  //未知的托盘图标类型
    APP_VOLUME_BUTTON
}ButtonType;

ButtonType btnType = DEVICE_VOLUME_BUTTON;
guint appnum = 0;
bool show = false;
DeviceSwitchWidget::DeviceSwitchWidget(QWidget *parent) : QWidget (parent)
{
//    scrollWid = new ScrollWitget(this);

    appScrollWidget = new ScrollWitget(this);
    devScrollWidget = new ScrollWitget(this);
    devWidget = new UkmediaDeviceWidget(this);
    appWidget = new ApplicationVolumeWidget(this);

    devScrollWidget->area->setWidget(devWidget);
    appScrollWidget->area->setWidget(appWidget);

    output_stream_list = new QStringList;
    input_stream_list = new QStringList;
    device_name_list = new QStringList;
    device_display_name_list = new QStringList;
    stream_control_list = new QStringList;
    //初始化matemixer
    if (mate_mixer_init() == FALSE) {
        qDebug() << "libmatemixer initialization failed, exiting";
    }
    //创建context
    context = mate_mixer_context_new();
    mate_mixer_context_set_app_name (context,_("Ukui Volume Control App"));//设置app名
    mate_mixer_context_set_app_id(context, GVC_APPLET_DBUS_NAME);
    mate_mixer_context_set_app_version(context,VERSION);
    mate_mixer_context_set_app_icon(context,"ukuimedia-volume-control");
    //打开context
    if G_UNLIKELY (mate_mixer_context_open(context) == FALSE) {
        g_warning ("Failed to connect to a sound system**********************");
    }

    context_set_property(this);
    g_signal_connect (G_OBJECT (context),
                     "notify::state",
                     G_CALLBACK (on_context_state_notify),
                     this);
    deviceSwitchWidgetInit();
    inputDeviceVisiable();
    connect(deviceBtn,SIGNAL(clicked()),this,SLOT(device_button_clicked_slot()));
    connect(appVolumeBtn,SIGNAL(clicked()),this,SLOT(appvolume_button_clicked_slot()));

    appWidget->setFixedSize(360,500);
    this->setMinimumSize(400,320);
    this->setMaximumSize(400,320);
    this->move(1507,775);
    devWidget->move(40,0);
    appWidget->move(40,0);
    appScrollWidget->move(40,0);
    devScrollWidget->move(40,0);
    devWidget->show();
    appWidget->hide();

    this->setStyleSheet("QWidget{width:400px;"
                        "height:320px;"
                        "background:rgba(14,19,22,1);"
                        "opacity:0.95;"
                       "border-radius:3px 3px 0px 0px;}");
    //    this->move(0,0);
//    appWidget->gridlayout->addWidget(appWidget->applicationLabel);
    qDebug() << 92;
    appScrollWidget->area->widget()->adjustSize();
    devScrollWidget->area->widget()->adjustSize();
}

/*初始化主界面*/
void DeviceSwitchWidget::deviceSwitchWidgetInit()
{
    const QSize iconSize(16,16);
    QWidget *deviceWidget = new QWidget(this);
    deviceWidget->setFixedSize(40,320);

    deviceBtn = new QPushButton(deviceWidget);
    appVolumeBtn = new QPushButton(deviceWidget);

    deviceBtn->setFocusPolicy(Qt::NoFocus);
    deviceBtn->setFixedSize(36,36);
    appVolumeBtn->setFixedSize(36,36);

    deviceBtn->setIconSize(iconSize);
    appVolumeBtn->setIconSize(iconSize);

    deviceBtn->setIcon(QIcon("/usr/share/ukui-media/img/device.svg"));
    appVolumeBtn->setIcon(QIcon("/usr/share/ukui-media/img/application.svg"));

    deviceBtn->move(2,10);
    appVolumeBtn->move(2,57);

    deviceBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                                "padding-left:0px;}"
                                "QPushButton::pressed{background:rgba(61,107,229,1);"
                                "border-radius:4px;}");
    appVolumeBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                                "padding-left:0px;}"
                                "QPushButton::hover{background:rgba(61,107,229,1);"
                                "border-radius:4px;}"
                                "QPushButton::pressed{background:rgba(61,107,229,1);"
                                "border-radius:4px;}");
}

/*点击切换设备按钮对应的槽函数*/
void DeviceSwitchWidget::device_button_clicked_slot()
{
    appWidget->hide();
    appScrollWidget->hide();
    devScrollWidget->show();
    devWidget->show();

    appVolumeBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                                "padding-left:0px;}");
    deviceBtn->setStyleSheet("QPushButton{background:rgba(61,107,229,1);"
                                 "border-radius:4px;}");
}

/*点击切换应用音量按钮对应的槽函数*/
void DeviceSwitchWidget::appvolume_button_clicked_slot()
{
//    appWidget->appLabel->move(20,23);
//    appWidget->noAppLabel->move(60,123);
    appScrollWidget->show();
    devScrollWidget->hide();
    appWidget->show();
    devWidget->hide();
    //切换按钮样式
    deviceBtn->setStyleSheet("QPushButton{background:transparent;border:0px;"
                                "padding-left:0px;}");
    appVolumeBtn->setStyleSheet("QPushButton{background:rgba(61,107,229,1);"
                             "border-radius:4px;}");
}

/*
 * context状态通知
*/
void DeviceSwitchWidget::on_context_state_notify (MateMixerContext *context,GParamSpec *pspec,DeviceSwitchWidget *w)
{
    MateMixerState state = mate_mixer_context_get_state (context);
    list_device(w,context);
    if (state == MATE_MIXER_STATE_READY) {

        update_icon_output(w->devWidget,context);
        update_icon_input(w->devWidget,context);
    }
    else if (state == MATE_MIXER_STATE_FAILED) {

    }
//    qDebug() << "设备列表为" << w->device_name_list->at(0) << w->device_name_list->size() << "stream size" << w->output_stream_list->at(0);
    //点击输出设备
//    connect(w->ui->outputDeviceCombobox,SIGNAL(currentIndexChanged(int)),w,SLOT(output_device_combox_index_changed_slot(int)));
    //点击输入设备
//    connect(w->ui->inputDeviceCombobox,SIGNAL(currentIndexChanged(int)),w,SLOT(input_device_combox_index_changed_slot(int)));

}

/*
    context 存储control增加
*/
void DeviceSwitchWidget::on_context_stored_control_added (MateMixerContext *context,const gchar *name,DeviceSwitchWidget *w)
{
    MateMixerStreamControl *control;
    MateMixerStreamControlMediaRole media_role;

    control = MATE_MIXER_STREAM_CONTROL (mate_mixer_context_get_stored_control (context, name));
    if (G_UNLIKELY (control == nullptr))
        return;

    media_role = mate_mixer_stream_control_get_media_role (control);

    if (media_role == MATE_MIXER_STREAM_CONTROL_MEDIA_ROLE_EVENT)
        bar_set_stream_control (w, control);
}


/*
    当其他设备插入时添加这个stream
*/
void DeviceSwitchWidget::on_context_stream_added (MateMixerContext *context,const gchar *name,DeviceSwitchWidget *w)
{
    MateMixerStream *stream;
    MateMixerDirection direction;
//        GtkWidget         *bar;
    stream = mate_mixer_context_get_stream (context, name);
    qDebug() << "context stream 添加" << name;
    if (G_UNLIKELY (stream == nullptr))
        return;
    direction = mate_mixer_stream_get_direction (stream);

    /* If the newly added stream belongs to the currently selected device and
     * the test button is hidden, this stream may be the one to allow the
     * sound test and therefore we may need to enable the button */
    if (/*dialog->priv->hw_profile_combo != nullptr && */direction == MATE_MIXER_DIRECTION_OUTPUT) {
        MateMixerDevice *device1;
        MateMixerDevice *device2;

        device1 = mate_mixer_stream_get_device (stream);

        if (device1 == device2) {
            gboolean show_button;

        }
    }
    add_stream (w, stream,context);
    qDebug() << 194;
}

/*
列出设备
*/
void DeviceSwitchWidget::list_device(DeviceSwitchWidget *w,MateMixerContext *context)
{
    const GList *list;
    const GList *stream_list;

    list = mate_mixer_context_list_streams (context);

    while (list != nullptr) {
        add_stream (w, MATE_MIXER_STREAM (list->data),context);
        MateMixerStream *s = MATE_MIXER_STREAM(list->data);
        const gchar *stream_name = mate_mixer_stream_get_name(s);

        MateMixerDirection direction = mate_mixer_stream_get_direction(s);
        if (direction == MATE_MIXER_DIRECTION_OUTPUT) {

            w->output_stream_list->append(stream_name);
            qDebug() << "输出stream 名为" << mate_mixer_stream_get_name(s);
        }
        else if (direction == MATE_MIXER_DIRECTION_INPUT) {
            w->input_stream_list->append(stream_name);
            qDebug() << "输入stream 名为" << mate_mixer_stream_get_name(s);
        }
        list = list->next;
    }

    list = mate_mixer_context_list_devices (context);

    while (list != nullptr) {
//                    add_device (self, MATE_MIXER_DEVICE (list->data));
        QString str =  mate_mixer_device_get_label(MATE_MIXER_DEVICE (list->data));

        const gchar *dis_name = mate_mixer_device_get_name(MATE_MIXER_DEVICE (list->data));
        w->device_name_list->append(dis_name);
        qDebug() << "设备名为" << str << dis_name;
        list = list->next;
    }

}

void DeviceSwitchWidget::add_stream (DeviceSwitchWidget *w, MateMixerStream *stream,MateMixerContext *context)
{

    const gchar *speakers = nullptr;
    const GList *controls;
    gboolean is_default = FALSE;
    MateMixerDirection direction;

    direction = mate_mixer_stream_get_direction (stream);
    if (direction == MATE_MIXER_DIRECTION_INPUT) {
        MateMixerStream *input;
        input = mate_mixer_context_get_default_input_stream (context);
        if (stream == input) {
            bar_set_stream (w, stream);
            is_default = TRUE;
        }
    }
    else if (direction == MATE_MIXER_DIRECTION_OUTPUT) {
        MateMixerStream        *output;
        MateMixerStreamControl *control;
        output = mate_mixer_context_get_default_output_stream (context);
        control = mate_mixer_stream_get_default_control (stream);

        if (stream == output) {
            update_output_settings(w,control);
            qDebug() << "stream is default";
            bar_set_stream (w, stream);

            is_default = TRUE;
        }
//                model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->priv->output_treeview));

        if (G_LIKELY (control != nullptr)) {
//            speakers = utils.gvc_channel_map_to_pretty_string (control);

        }
    }

    controls = mate_mixer_stream_list_controls (stream);

    while (controls != nullptr) {
        MateMixerStreamControl    *control = MATE_MIXER_STREAM_CONTROL (controls->data);
        MateMixerStreamControlRole role;

        role = mate_mixer_stream_control_get_role (control);
        if (role == MATE_MIXER_STREAM_CONTROL_ROLE_APPLICATION) {
            qDebug() << " 283 application";
            add_application_control (w, control);
        }
        controls = controls->next;
    }

    // XXX find a way to disconnect when removed
    g_signal_connect (G_OBJECT (stream),
                      "control-added",
                      G_CALLBACK (on_stream_control_added),
                      w);
    g_signal_connect (G_OBJECT (stream),
                      "control-removed",
                      G_CALLBACK (on_stream_control_removed),
                      w);
}

/*
    添加应用音量控制
*/
void DeviceSwitchWidget::add_application_control (DeviceSwitchWidget *w, MateMixerStreamControl *control)
{
    MateMixerStream *stream;
    MateMixerStreamControlMediaRole media_role;
    MateMixerAppInfo *info;
    guint app_count;
    MateMixerDirection direction = MATE_MIXER_DIRECTION_UNKNOWN;
    const gchar *app_id;
    const gchar *app_name;
    const gchar *app_icon;
    appnum++;
    app_count = appnum;
    guint volume = mate_mixer_stream_control_get_volume(control);

    media_role = mate_mixer_stream_control_get_media_role (control);

    /* Add stream to the applications page, but make sure the stream qualifies
     * for the inclusion */
    info = mate_mixer_stream_control_get_app_info (control);
    if (info == nullptr)
        return;

    /* Skip streams with roles we don't care about */
    if (media_role == MATE_MIXER_STREAM_CONTROL_MEDIA_ROLE_EVENT ||
        media_role == MATE_MIXER_STREAM_CONTROL_MEDIA_ROLE_TEST ||
        media_role == MATE_MIXER_STREAM_CONTROL_MEDIA_ROLE_ABSTRACT ||
        media_role == MATE_MIXER_STREAM_CONTROL_MEDIA_ROLE_FILTER)
            return;

    app_id = mate_mixer_app_info_get_id (info);

    /* These applications may have associated streams because they do peak
     * level monitoring, skip these too */
    if (!g_strcmp0 (app_id, "org.mate.VolumeControl") ||
        !g_strcmp0 (app_id, "org.gnome.VolumeControl") ||
        !g_strcmp0 (app_id, "org.PulseAudio.pavucontrol"))
            return;

    QString app_icon_name = mate_mixer_app_info_get_icon(info);
    qDebug() << "应用音量+++";
    app_name = mate_mixer_app_info_get_name (info);
    //添加应用音量
    add_app_to_tableview(w,appnum,app_name,app_icon_name,control);

//    w->ui->appVolumeTableView->show();
    if (app_name == nullptr)
        app_name = mate_mixer_stream_control_get_label (control);
    if (app_name == nullptr)
        app_name = mate_mixer_stream_control_get_name (control);
    if (G_UNLIKELY (app_name == nullptr))
        return;

    /* By default channel bars use speaker icons, use microphone icons
     * instead for recording applications */
    stream = mate_mixer_stream_control_get_stream (control);
    if (stream != nullptr)
        direction = mate_mixer_stream_get_direction (stream);

    if (direction == MATE_MIXER_DIRECTION_INPUT) {
    }
    app_icon = mate_mixer_app_info_get_icon (info);
    if (app_icon == nullptr) {
        if (direction == MATE_MIXER_DIRECTION_INPUT)
            app_icon = "audio-input-microphone";
        else
            app_icon = "applications-multimedia";
    }

    bar_set_stream_control (w, control);
}

void DeviceSwitchWidget::on_stream_control_added (MateMixerStream *stream,const gchar *name,DeviceSwitchWidget *w)
{
    MateMixerStreamControl    *control;
    MateMixerStreamControlRole role;
    qDebug() << "add stream control" << name;
    w->stream_control_list->append(name);
    control = mate_mixer_stream_get_control (stream, name);
    if G_UNLIKELY (control == nullptr)
        return;

    role = mate_mixer_stream_control_get_role (control);
    if (role == MATE_MIXER_STREAM_CONTROL_ROLE_APPLICATION) {
        qDebug() << " 386 application";
        add_application_control (w, control);
    }
}

/*
    移除control
*/
void DeviceSwitchWidget::on_stream_control_removed (MateMixerStream *stream,const gchar *name,DeviceSwitchWidget *w)
{
    MateMixerStreamControl *control;
    qDebug() << "stream control remove" << name;
    int i = w->stream_control_list->indexOf(name);
    /* No way to be sure that it is an application control, but we don't have
     * any other than application bars that could match the name */
    remove_application_control (w, name);
}

void DeviceSwitchWidget::remove_application_control (DeviceSwitchWidget *w,const gchar *name)
{
    g_debug ("Removing application stream %s", name);
        /* We could call bar_set_stream_control here, but that would pointlessly
         * invalidate the channel bar, so just remove it ourselves */
    int index = w->appWidget->app_volume_list->indexOf(name);
    int i = w->stream_control_list->indexOf(name);

    w->stream_control_list->removeAt(i);
    qDebug() << "xiabiaowei" << i << "double " << w->stream_control_list->indexOf(name) << "size" << w->stream_control_list->size();

    //当播放音乐的应用程序退出后删除该项
    QLayoutItem *item = w->appWidget->gridlayout->takeAt(i);
    item->widget()->setParent(0);
    delete  item;

    w->appWidget->gridlayout->update();
//    w->standItemModel->removeRow(i);
    if (appnum <= 0) {
        g_warn_if_reached ();
        appnum = 1;
    }
    appnum--;
    if (appnum <= 0)
        w->appWidget->noAppLabel->show();
    else
        w->appWidget->noAppLabel->hide();

}

void DeviceSwitchWidget::add_app_to_tableview(DeviceSwitchWidget *w,int appnum, const gchar *app_name,QString app_icon_name,MateMixerStreamControl *control)
{
    //设置QTableView每行的宽度
    //获取应用静音状态及音量
    int volume = 0;
    gboolean is_mute = false;
    gdouble normal = 0.0;
    is_mute = mate_mixer_stream_control_get_mute(control);
    volume = mate_mixer_stream_control_get_volume(control);
    normal = mate_mixer_stream_control_get_normal_volume(control);

    int display_volume = 100 * volume / normal;

    //设置应用的图标
    QString iconName = "/usr/share/applications/";
    iconName.append(app_icon_name);
    iconName.append(".desktop");
    XdgDesktopFile xdg;
    xdg.load(iconName);
    QIcon i=xdg.icon();
    GError **error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile *keyflie = g_key_file_new();
    QByteArray fpbyte = iconName.toLocal8Bit();
    char *filepath = "/usr/share/applications";//fpbyte.data();
    g_key_file_load_from_file(keyflie,iconName.toLocal8Bit(),flags,error);
    char *icon_str = g_key_file_get_locale_string(keyflie,"Desktop Entry","Icon",nullptr,nullptr);
    QIcon icon = QIcon::fromTheme(QString::fromLocal8Bit(icon_str));
    w->appWidget->app_volume_list->append(app_icon_name);

    //widget显示应用音量
    QWidget *app_widget = new QWidget(w->appWidget);
    app_widget->setFixedSize(340,40);
    QHBoxLayout *hlayout1 = new QHBoxLayout(app_widget);
    QHBoxLayout *hlayout2 = new QHBoxLayout();
    QVBoxLayout *vlayout = new QVBoxLayout();

    QWidget *wid1 = new QWidget(app_widget);
    QWidget *wid2 = new QWidget(app_widget);

    w->appWidget->appLabel = new QLabel(app_widget);
    w->appWidget->appIconBtn = new QPushButton(app_widget);
    w->appWidget->appIconLabel = new QLabel(app_widget);
    w->appWidget->appVolumeLabel = new QLabel(app_widget);
    w->appWidget->appSlider = new UkmediaDeviceSlider(app_widget);
    w->appWidget->appSlider->setOrientation(Qt::Horizontal);

    QSpacerItem *item1 = new QSpacerItem(16,20);
    QSpacerItem *item2 = new QSpacerItem(2,20);
    QSpacerItem *item3 = new QSpacerItem(2,20);
    QSpacerItem *item4 = new QSpacerItem(2,20);
    QSpacerItem *item5 = new QSpacerItem(2,20);
    QSpacerItem *item6 = new QSpacerItem(2,20);

    hlayout1->addWidget(w->appWidget->appSlider);
    hlayout1->addWidget(w->appWidget->appVolumeLabel);
    hlayout1->setSpacing(10);
    wid1->setLayout(hlayout1);
    hlayout1->setMargin(0);

    vlayout->addWidget(w->appWidget->appLabel);
    vlayout->addWidget(wid1);
    vlayout->setSpacing(6);
    wid2->setLayout(vlayout);
    vlayout->setMargin(0);

    hlayout2->addWidget(w->appWidget->appIconBtn);
    hlayout2->addItem(item1);
    hlayout2->addWidget(wid2);
    hlayout2->setSpacing(0);
    app_widget->setLayout(hlayout2);
    hlayout2->setMargin(0);
    app_widget->layout()->setSpacing(0);
    //添加widget到gridlayout中

    w->appWidget->gridlayout->addWidget(app_widget);
    w->appWidget->gridlayout->setMargin(0);

    app_widget->move(0,50+(appnum-1)*50);

    //设置每项的固定大小
    w->appWidget->appLabel->setFixedSize(88,14);
    w->appWidget->appIconBtn->setFixedSize(32,32);
    w->appWidget->appIconLabel->setFixedSize(24,24);
    w->appWidget->appVolumeLabel->setFixedSize(36,14);

    QSize icon_size(32,32);
    w->appWidget->appIconBtn->setIconSize(icon_size);
    w->appWidget->appIconBtn->setStyleSheet("QPushButton{background:transparent;border:0px;padding-left:0px;}");
    w->appWidget->appIconBtn->setIcon(icon);
    w->appWidget->appIconBtn->setFlat(true);
    w->appWidget->appIconBtn->setFocusPolicy(Qt::NoFocus);
    w->appWidget->appIconBtn->setEnabled(true);

    w->appWidget->appSlider->setMaximum(100);
    w->appWidget->appSlider->setFixedSize(220,20);

    QString appSliderStr = app_name;
    QString appLabelStr = app_name;
    QString appVolumeLabelStr = app_name;

    appSliderStr.append("Slider");
    appLabelStr.append("Label");
    appVolumeLabelStr.append("VolumeLabel");
    w->appWidget->appSlider->setObjectName(appSliderStr);
    w->appWidget->appLabel->setObjectName(appLabelStr);
    w->appWidget->appVolumeLabel->setObjectName(appVolumeLabelStr);
    //设置label 和滑动条的值
    w->appWidget->appLabel->setText(app_name);
    w->appWidget->appSlider->setValue(display_volume);
    w->appWidget->appVolumeLabel->setNum(display_volume);

    /*滑动条控制应用音量*/
    connect(w->appWidget->appSlider,&QSlider::valueChanged,[=](int value){
        QSlider *s = w->findChild<QSlider*>(appSliderStr);
        s->setValue(value);
        QLabel *l = w->findChild<QLabel*>(appVolumeLabelStr);
        l->setNum(value);

        int v = value*65536/100 + 0.5;
        mate_mixer_stream_control_set_volume(control,(int)v);
//        qDebug() << "滚动滑动条" << value << appVolumeLabelStr;
    });
    /*应用音量同步*/
    g_signal_connect (G_OBJECT (control),
                     "notify::volume",
                     G_CALLBACK (update_app_volume),
                     w);

    connect(w,&DeviceSwitchWidget::app_volume_changed,[=](bool is_mute,int volume,const gchar *app_name){
        QString slider_str = app_name;
        slider_str.append("Slider");
        QSlider *s = w->findChild<QSlider*>(slider_str);
        s->setValue(volume);
    });

    if (appnum <= 0)
        w->appWidget->noAppLabel->show();
    else
        w->appWidget->noAppLabel->hide();

    w->appWidget->gridlayout->setMargin(0);
    w->appWidget->gridlayout->setSpacing(0);
    w->appWidget->gridlayout->setAlignment(app_widget,Qt::AlignCenter);

    //设置布局的垂直间距以及设置gridlayout四周的间距
    w->appWidget->gridlayout->setVerticalSpacing(200);
//    w->appWidget->gridlayout->setContentsMargins(20,60,20,200);

    w->appWidget->appLabel->setStyleSheet("QLabel{background:transparent;"
                                          "border:0px;"
                                          "color:#ffffff;"
                                          "font-size:14px;}");

}

/*
    应用音量滑动条滚动事件
*/
void DeviceSwitchWidget::app_slider_changed_slot(int volume)
{
//    qDebug() << "滚动滑动条" << volume;
    mate_mixer_stream_control_set_volume;

}

/*
    同步应用音量
*/
void DeviceSwitchWidget::update_app_volume(MateMixerStreamControl *control, GParamSpec *pspec, DeviceSwitchWidget *w)
{
    Q_UNUSED(pspec);

    guint value = mate_mixer_stream_control_get_volume(control);
    guint volume ;
    volume = guint(value*100/65536.0+0.5);
    bool is_mute = mate_mixer_stream_control_get_mute(control);
    MateMixerStreamControlFlags control_flags = mate_mixer_stream_control_get_flags(control);
    MateMixerAppInfo *info = mate_mixer_stream_control_get_app_info(control);
    const gchar *app_name = mate_mixer_app_info_get_name(info);
    Q_EMIT w->app_volume_changed(is_mute,volume,app_name);
//    qDebug() << "发送信号同步音量值" << is_mute <<volume ;

    //设置声音标签图标
    QPixmap pix;

    if (volume <= 0) {
        pix = QPixmap("/usr/share/ukui-media/img/audio-mute.png");
//        w->appIconLabel->setPixmap(pix);
    }
    else if (volume > 0 && volume <= 33) {
        pix = QPixmap("/usr/share/ukui-media/img/audio-low.png");
//        w->appIconLabel->setPixmap(pix);
    }
    else if (volume >33 && volume <= 66) {
        pix = QPixmap("/usr/share/ukui-media/img/audio-medium.png");
//        w->appIconLabel->setPixmap(pix);
    }
    else {
        pix = QPixmap("/usr/share/ukui-media/img/audio-high.png");
//        w->appIconLabel->setPixmap(pix);
    }
    //静音可读并且处于静音
    if ((control_flags & MATE_MIXER_STREAM_CONTROL_MUTE_WRITABLE) ) {
    }
    if (control_flags & MATE_MIXER_STREAM_CONTROL_VOLUME_WRITABLE) {
        //设置滑动条的值
//        Q_EMIT->emitVolume(volume);
    }
}

void DeviceSwitchWidget::app_volume_changed_slot(bool is_mute,int volume,const gchar *app_name)
{

}

/*
    连接context，处理不同信号
*/
void DeviceSwitchWidget::set_context(DeviceSwitchWidget *w,MateMixerContext *context)
{
    qDebug() << "set contetx";
    g_signal_connect (G_OBJECT (context),
                      "stream-added",
                      G_CALLBACK (on_context_stream_added),
                      w);

    g_signal_connect (G_OBJECT (context),
                    "stream-removed",
                    G_CALLBACK (on_context_stream_removed),
                    w);

    g_signal_connect (G_OBJECT (context),
                    "device-added",
                    G_CALLBACK (on_context_device_added),
                    w);
    g_signal_connect (G_OBJECT (context),
                    "device-removed",
                    G_CALLBACK (on_context_device_removed),
                    w);

    g_signal_connect (G_OBJECT (context),
                    "notify::default-input-stream",
                    G_CALLBACK (on_context_default_input_stream_notify),
                    w);
    g_signal_connect (G_OBJECT (context),
                    "notify::default-output-stream",
                    G_CALLBACK (on_context_default_output_stream_notify),
                    w);

    g_signal_connect (G_OBJECT (context),
                    "stored-control-added",
                    G_CALLBACK (on_context_stored_control_added),
                    w);
    g_signal_connect (G_OBJECT (context),
                    "stored-control-removed",
                    G_CALLBACK (on_context_stored_control_removed),
                    w);

}

/*
    remove stream
*/
void DeviceSwitchWidget::on_context_stream_removed (MateMixerContext *context,const gchar *name,DeviceSwitchWidget *w)
{
    qDebug() << "context stream 移除" << name;
    remove_stream (w, name);
}

/*
    移除stream
*/
void DeviceSwitchWidget::remove_stream (DeviceSwitchWidget *w, const gchar *name)
{

    MateMixerStream *stream = mate_mixer_context_get_stream(w->context,name);
    MateMixerDirection direction = mate_mixer_stream_get_direction(stream);
    bool status;
    if (direction == MATE_MIXER_DIRECTION_INPUT) {
        status = w->input_stream_list->removeOne(name);
        qDebug() << "移除input stream list " << status;
    }
    else if (direction == MATE_MIXER_DIRECTION_OUTPUT) {
        status = w->output_stream_list->removeOne(name);
        qDebug() << "移除input stream list " << status;
    }
        if (w->appWidget->app_volume_list != nullptr) {

                bar_set_stream (w,  NULL);
        }

}

/*
    context 添加设备并设置到单选框
*/
void DeviceSwitchWidget::on_context_device_added (MateMixerContext *context, const gchar *name, DeviceSwitchWidget *w)
{
    MateMixerDevice *device;
    device = mate_mixer_context_get_device (context, name);
    qDebug() << "748 context 设备添加" << mate_mixer_device_get_label(device) <<name;

    if (G_UNLIKELY (device == nullptr))
            return;
    add_device (w, device);
}

/*
    添加设备
*/
void DeviceSwitchWidget::add_device (DeviceSwitchWidget *w, MateMixerDevice *device)
{
    const gchar *name;
    const gchar *label;
    gchar *status;
    const gchar *profile_label = nullptr;
    MateMixerSwitch *profile_switch;

    name  = mate_mixer_device_get_name (device);
    label = mate_mixer_device_get_label (device);
    w->device_name_list->append(name);
    //添加设备到组合框

}

/*
    移除设备
*/
void DeviceSwitchWidget::on_context_device_removed (MateMixerContext *context,const gchar *name,DeviceSwitchWidget *w)
{
    int  count = 0;
    MateMixerDevice *dev = mate_mixer_context_get_device(context,name);
    QString str = mate_mixer_device_get_label(dev);
    do {
        if (name == w->device_name_list->at(count)) {
//            qDebug() << "context设备移除" << name << "移除的设备名为" << w->device_name_list->at(count);
            qDebug() << "device remove";
            w->device_name_list->removeAt(count);
            break;
        }
        count++;
        if (count > w->device_name_list->size()) {
            qDebug() << "device error";
            break;
        }
    }while(1);

    if (dev == nullptr)
        qDebug() << "device is null";
}

/*
    默认输入流通知
*/
void DeviceSwitchWidget::on_context_default_input_stream_notify (MateMixerContext *context,GParamSpec *pspec,DeviceSwitchWidget *w)
{
    MateMixerStream *stream;

    g_debug ("Default input stream has changed");
    stream = mate_mixer_context_get_default_input_stream (context);

    set_input_stream (w, stream);
}

void DeviceSwitchWidget::set_input_stream (DeviceSwitchWidget *w, MateMixerStream *stream)
{
    MateMixerSwitch *swtch;
    MateMixerStreamControl *control;

//        control = gvc_channel_bar_get_control (GVC_CHANNEL_BAR (dialog->priv->input_bar));
    if (control != nullptr) {
        mate_mixer_stream_control_set_monitor_enabled (control, FALSE);
    }

    bar_set_stream (w, stream);

    if (stream != nullptr) {
        const GList *controls;

        controls = mate_mixer_context_list_stored_controls (w->context);

        /* Move all stored controls to the newly selected default stream */
        while (controls != nullptr) {
            MateMixerStream *parent;

            control = MATE_MIXER_STREAM_CONTROL (controls->data);
            parent  = mate_mixer_stream_control_get_stream (control);

            /* Prefer streamless controls to stay the way they are, forcing them to
             * a particular owning stream would be wrong for eg. event controls */
            if (parent != nullptr && parent != stream) {
                MateMixerDirection direction =
                    mate_mixer_stream_get_direction (parent);

                if (direction == MATE_MIXER_DIRECTION_INPUT)
                    mate_mixer_stream_control_set_stream (control, stream);
            }
            controls = controls->next;
        }

        /* Enable/disable the peak level monitor according to mute state */
        g_signal_connect (G_OBJECT (stream),
                          "notify::mute",
                          G_CALLBACK (on_stream_control_mute_notify),
                          w);
    }

}

/*
    control 静音通知
*/
void DeviceSwitchWidget::on_stream_control_mute_notify (MateMixerStreamControl *control,GParamSpec *pspec,DeviceSwitchWidget *dialog)
{
    /* Stop monitoring the input stream when it gets muted */
    if (mate_mixer_stream_control_get_mute (control) == TRUE)
        mate_mixer_stream_control_set_monitor_enabled (control, FALSE);
    else
        mate_mixer_stream_control_set_monitor_enabled (control, TRUE);
}

/*
    默认输出流通知
*/
void DeviceSwitchWidget::on_context_default_output_stream_notify (MateMixerContext *context,GParamSpec *pspec,DeviceSwitchWidget *w)
{
    MateMixerStream *stream;
    qDebug() << "默认的输出stream改变";
    stream = mate_mixer_context_get_default_output_stream (context);
//    update_icon_output(w,context);
    set_output_stream (w, stream);
}

/*
    移除存储control
*/
void DeviceSwitchWidget::on_context_stored_control_removed (MateMixerContext *context,const gchar *name,DeviceSwitchWidget *w)
{
//        GtkWidget *bar;

//        bar = g_hash_table_lookup (dialog->priv->bars, name);

    if (w->appWidget->app_volume_list != nullptr) {
            /* We only use a stored control in the effects bar */
//                if (G_UNLIKELY (bar != dialog->priv->effects_bar)) {
//                        g_warn_if_reached ();
//                        return;
//                }

            bar_set_stream (w, NULL);
    }
}

/*
 * context设置属性
*/
void DeviceSwitchWidget::context_set_property(DeviceSwitchWidget *w)//,guint prop_id,const GValue *value,GParamSpec *pspec)
{
    set_context(w,w->context);
}

/*
    输出音量控制
*/
void DeviceSwitchWidget::output_volume_slider_changed_slot(int volume)
{

//    stream = mate_mixer_context_get_stream(context)
    QString percent = QString::number(volume);
//    ui->opVolumePercentLabel->setText(percent);

}

/*
    输入音量控制
*/
void DeviceSwitchWidget::input_volume_slider_changed_slot(int volume)
{
    QString percent = QString::number(volume);
//    ui->ipVolumePercentLabel->setText(percent);
}

/*
    更新输入音量及图标
*/
void DeviceSwitchWidget::update_icon_input (UkmediaDeviceWidget *w,MateMixerContext *context)
{
    MateMixerStream        *stream;
    MateMixerStreamControl *control = nullptr;
    const gchar *app_id;
    gboolean show = FALSE;

    stream = mate_mixer_context_get_default_input_stream (context);

    const GList *inputs =mate_mixer_stream_list_controls(stream);
    control = mate_mixer_stream_get_default_control(stream);

    //初始化滑动条的值
    int volume = mate_mixer_stream_control_get_volume(control);
    int value = volume *100 /65536.0+0.5;
    w->inputDeviceSlider->setValue(value);
    QString percent = QString::number(value);
    w->inputVolumeLabel->setText(percent);
//    w->ui->input_icon_btn->setFocusPolicy(Qt::NoFocus);
//    w->ui->input_icon_btn->setStyleSheet("QPushButton{background:transparent;border:0px;padding-left:0px;}");

    while (inputs != nullptr) {
        MateMixerStreamControl *input = MATE_MIXER_STREAM_CONTROL (inputs->data);
        MateMixerStreamControlRole role = mate_mixer_stream_control_get_role (input);
        if (role == MATE_MIXER_STREAM_CONTROL_ROLE_APPLICATION) {
            MateMixerAppInfo *app_info = mate_mixer_stream_control_get_app_info (input);
            app_id = mate_mixer_app_info_get_id (app_info);
            if (app_id == nullptr) {
                /* A recording application which has no
                 * identifier set */
                g_debug ("Found a recording application control %s",
                    mate_mixer_stream_control_get_label (input));

                if G_UNLIKELY (control == nullptr) {
                    /* In the unlikely case when there is no
                     * default input control, use the application
                     * control for the icon */
                    control = input;
                }
                show = TRUE;
                break;
            }

            if (strcmp (app_id, "org.mate.VolumeControl") != 0 &&
                strcmp (app_id, "org.gnome.VolumeControl") != 0 &&
                strcmp (app_id, "org.PulseAudio.pavucontrol") != 0) {
                    g_debug ("Found a recording application %s", app_id);

                    if G_UNLIKELY (control == nullptr)
                            control = input;

                    show = TRUE;
                    break;
            }
        }
        inputs = inputs->next;
    }

        if (show == TRUE)
                g_debug ("Input icon enabled");
        else
                g_debug ("There is no recording application, input icon disabled");

        connect(w->inputDeviceSlider,&QSlider::valueChanged,[=](int value){
            QString percent;

            percent = QString::number(value);
            mate_mixer_stream_control_set_mute(control,FALSE);
            int volume = value*65536/100;
            gboolean ok = mate_mixer_stream_control_set_volume(control,volume);
            w->inputVolumeLabel->setText(percent);
        });
        gvc_stream_status_icon_set_control (w, control);

        if (control != nullptr) {
                g_debug ("Output icon enabled");
                qDebug() << "control is not null";
        }
        else {
                g_debug ("There is no output stream/control, output icon disabled");
                qDebug() << "control is  null";
        }
}

/*
    更新输出音量及图标
*/
void DeviceSwitchWidget::update_icon_output (UkmediaDeviceWidget *w,MateMixerContext *context)
{
    MateMixerStream        *stream;
    MateMixerStreamControl *control = nullptr;

    stream = mate_mixer_context_get_default_output_stream (context);
    if (stream != nullptr)
        control = mate_mixer_stream_get_default_control (stream);

    gvc_stream_status_icon_set_control (w, control);
    //初始化滑动条的值
    int volume = mate_mixer_stream_control_get_volume(control);
    int value = volume *100 /65536.0+0.5;
    w->outputDeviceSlider->setValue(value);
    QString percent = QString::number(value);

    w->outputVolumeLabel->setText(percent);

    //输出音量控制
    //输出滑动条和音量控制
    connect(w->outputDeviceSlider,&QSlider::valueChanged,[=](int value){
        QString percent;

        percent = QString::number(value);
        mate_mixer_stream_control_set_mute(control,FALSE);
        int volume = value*65536/100;
        gboolean ok = mate_mixer_stream_control_set_volume(control,volume);
        w->outputVolumeLabel->setText(percent);
    });
    if (control != nullptr) {
            g_debug ("Output icon enabled");
    }
    else {
            g_debug ("There is no output stream/control, output icon disabled");
    }
}

void DeviceSwitchWidget::gvc_stream_status_icon_set_control (UkmediaDeviceWidget *w,MateMixerStreamControl *control)
{
    g_signal_connect ( G_OBJECT (control),
                      "notify::volume",
                      G_CALLBACK (on_stream_control_volume_notify),
                      w);
    g_signal_connect (G_OBJECT (control),
                      "notify::mute",
                      G_CALLBACK (on_stream_control_mute_notify),
                      w);

    MateMixerDirection direction = mate_mixer_stored_control_get_direction((MateMixerStoredControl *)control);
    if (direction == MATE_MIXER_DIRECTION_OUTPUT)
        qDebug() << "output******************";
    else if (direction == MATE_MIXER_DIRECTION_INPUT) {
        qDebug() << "input*****************";
    }
//    connect(w->ui->opVolumeSlider,SIGNAL(valueChanged(int)),w,SLOT(output_volume_slider_changed_slot(int)));

        if (control != nullptr)
//                g_object_ref (control);

        if (control != nullptr) {
//                g_signal_handlers_disconnect_by_func (G_OBJECT (control),
//                                                      G_CALLBACK (on_stream_control_volume_notify),
//                                                     w);
//                g_signal_handlers_disconnect_by_func (G_OBJECT (control),
//                                                      G_CALLBACK (on_stream_control_mute_notify),
//                                                      w);

//                g_object_unref (control);
        }

//        icon->priv->control = control;

//        if (icon->priv->control != nullptr) {

                // XXX when no stream set some default icon and "unset" dock
//                update_icon (icon);
//        }

//        gvc_channel_bar_set_control (GVC_CHANNEL_BAR (icon->priv->bar), icon->priv->control);

//        g_object_notify_by_pspec (G_OBJECT (icon), properties[PROP_CONTROL]);
}

/*
    stream control 声音通知
*/
void DeviceSwitchWidget::on_stream_control_volume_notify (MateMixerStreamControl *control,GParamSpec *pspec,UkmediaDeviceWidget *w)
{
    MateMixerStreamControlFlags flags;
    gboolean muted = FALSE;
    gdouble decibel = 0;
    guint volume = 0;
    guint normal = 0;
    QString decscription;

//    qDebug() << "control" << mate_mixer_stream_control_get_name() << volume;
    if (control != nullptr)
        flags = mate_mixer_stream_control_get_flags(control);

    if(flags&MATE_MIXER_STREAM_CONTROL_MUTE_READABLE)
        muted = mate_mixer_stream_control_get_mute(control);

    if (flags&MATE_MIXER_STREAM_CONTROL_VOLUME_READABLE) {
        volume = mate_mixer_stream_control_get_volume(control);
        normal = mate_mixer_stream_control_get_normal_volume(control);
    }

    if (flags&MATE_MIXER_STREAM_CONTROL_HAS_DECIBEL)
        decibel = mate_mixer_stream_control_get_decibel(control);
//        update_icon (icon);
    decscription = mate_mixer_stream_control_get_label(control);

    MateMixerStream *stream = mate_mixer_stream_control_get_stream(control);
    MateMixerDirection direction = mate_mixer_stream_get_direction(stream);

    //设置输出滑动条的值
    int value = volume*100/65536.0 + 0.5;
    if (direction == MATE_MIXER_DIRECTION_OUTPUT) {

        w->outputDeviceSlider->setValue(value);
    }
    else if (direction == MATE_MIXER_DIRECTION_INPUT) {
//        qDebug() << "输入" << value;
        w->inputDeviceSlider->setValue(value);
    }

//    qDebug() << "音量值为" << volume << value;



}


/*
    更新输出设置
*/
void DeviceSwitchWidget::update_output_settings (DeviceSwitchWidget *w,MateMixerStreamControl *control)
{
    MateMixerStreamControlFlags flags;
    flags = mate_mixer_stream_control_get_flags(control);

    if (flags & MATE_MIXER_STREAM_CONTROL_CAN_BALANCE) {
//        gvc_balance_bar_set_property(w,control);
    }

}

void DeviceSwitchWidget::set_output_stream (DeviceSwitchWidget *w, MateMixerStream *stream)
{
        MateMixerSwitch        *swtch;
        MateMixerStreamControl *control;
        qDebug() << "set output stream";
        bar_set_stream (w,stream);

        if (stream != NULL) {
                const GList *controls;

                controls = mate_mixer_context_list_stored_controls (w->context);

                /* Move all stored controls to the newly selected default stream */
                while (controls != NULL) {
                        MateMixerStream        *parent;
                        MateMixerStreamControl *control;

                        control = MATE_MIXER_STREAM_CONTROL (controls->data);
                        parent  = mate_mixer_stream_control_get_stream (control);

                        /* Prefer streamless controls to stay the way they are, forcing them to
                         * a particular owning stream would be wrong for eg. event controls */
                        if (parent != NULL && parent != stream) {
                                MateMixerDirection direction =
                                        mate_mixer_stream_get_direction (parent);

                                if (direction == MATE_MIXER_DIRECTION_OUTPUT)
                                        mate_mixer_stream_control_set_stream (control, stream);
                        }
                        controls = controls->next;
                }
        }

//        model = gtk_tree_view_get_model (GTK_TREE_VIEW (dialog->priv->output_treeview));
        update_output_stream_list (w, stream);
        update_output_settings(w,control);
}

/*
    更新输出stream 列表
*/
void DeviceSwitchWidget::update_output_stream_list(DeviceSwitchWidget *w,MateMixerStream *stream)
{
    const gchar *name = NULL;
    if (stream != nullptr) {
        name = mate_mixer_stream_get_name(stream);
        w->output_stream_list->append(name);
        qDebug() << "更新输出stream名为" << name;
    }
}

/*
    bar设置stream
*/
void DeviceSwitchWidget::bar_set_stream (DeviceSwitchWidget  *w,MateMixerStream *stream)
{
        MateMixerStreamControl *control = NULL;

        if (stream != NULL)
                control = mate_mixer_stream_get_default_control (stream);

        bar_set_stream_control (w, control);
        qDebug() << 1201;
}

void DeviceSwitchWidget::bar_set_stream_control (DeviceSwitchWidget *w,MateMixerStreamControl *control)
{
        const gchar *name;
        MateMixerStreamControl *previous;

        if (control != NULL) {
                name = mate_mixer_stream_control_get_name (control);
                qDebug() << "********control" << name;

        } else
            qDebug() << "set true";
}

void DeviceSwitchWidget::inputDeviceVisiable()
{
    //设置麦克风托盘图标是否可见
    MateMixerStreamControl *control;
    const gchar *app_id;
//    const GList *inputs = mate_mixer_stream_list_controls(widget->inputStream);
//    control = mate_mixer_stream_get_default_control(widget->inputStream);

//    while (inputs != NULL) {
//        MateMixerStreamControl *input = MATE_MIXER_STREAM_CONTROL (inputs->data);
//        MateMixerStreamControlRole role = mate_mixer_stream_control_get_role(input);

//        if (role == MATE_MIXER_STREAM_CONTROL_ROLE_APPLICATION) {
//            MateMixerAppInfo *app_info = mate_mixer_stream_control_get_app_info (input);
//            app_id = mate_mixer_app_info_get_id (app_info);
//            if (app_id == NULL) {
//               /* A recording application which has no
//                * identifier set */
//                g_debug ("Found a recording application control %s",
//                         mate_mixer_stream_control_get_label (input));
//                if G_UNLIKELY (control == NULL) {
//                       /* In the unlikely case when there is no
//                        * default input control, use the application
//                        * control for the icon */
//                    control = input;
//                }
//                show = TRUE;
//                break;
//            }
//            if (strcmp (app_id, "org.mate.VolumeControl") != 0 &&
//                strcmp (app_id, "org.gnome.VolumeControl") != 0 &&
//                strcmp (app_id, "org.PulseAudio.pavucontrol") != 0) {
//                g_debug ("Found a recording application %s", app_id);
//                if G_UNLIKELY (control == NULL)
//                    control = input;
//                show = TRUE;
//                break;
//            }
//        }
//        inputs = inputs->next;
//    }
//    if (show == TRUE)
//        g_debug ("Input icon enabled");
//    else
        g_debug ("There is no recording application, input icon disabled");
    this->devWidget->inputWidget->show();
}

DeviceSwitchWidget::~DeviceSwitchWidget()
{


}