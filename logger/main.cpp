#include <QDebug>
#include <QString>
#include <QThread>

#include "hidapi.h"

static QTextStream cout(stdout);

#define MAX_STR 255

#define HID_VID 0x2803

static void hidstr_print(char *head, wchar_t *hidstr)
{
    cout << head << QString::fromWCharArray(hidstr) << endl;
}

int main(int argc, char *argv[])
{
    Q_UNUSED(argc)
    Q_UNUSED(argv)

    struct hid_device_info *devs, *cur_dev;
    hid_device *handle;
    wchar_t wstr[MAX_STR];
    int res = 0;

    system("chcp 1251");

    res = hid_init();
    if (res != 0) { qDebug("res: %d", res); }

    devs = hid_enumerate(HID_VID, 0x0000);
    if (devs != nullptr)
    {
        cur_dev = devs;
        while (cur_dev)
        {
            if (cur_dev->vendor_id == HID_VID)
            {
                cout << "Found: "   << '\n';
                cout << "PID: 0x"   << hex << cur_dev->product_id << '\n';
                cout << "Vendor: "  << QString::fromWCharArray(cur_dev->manufacturer_string) << '\n';
                cout << "Device: "  << QString::fromWCharArray(cur_dev->product_string) << '\n';
                cout << "SN: "      << QString::fromWCharArray(cur_dev->serial_number);
                cout << endl;
                break;
            }
            else
            {
                cur_dev = cur_dev->next;
            }
        }
        hid_free_enumeration(devs);
    }
    if ((devs != nullptr) && (cur_dev->vendor_id == HID_VID))
    {
        handle = hid_open(HID_VID, cur_dev->product_id, NULL);

        unsigned char buf[65];
        QByteArray log = QByteArray(0);

        while (1)
        {
            memset(buf, 0, sizeof(buf));
            res = hid_read(handle, buf, 64);
            if (res != -1)
            {
                static bool start = true;

                /*
                {
                    for (int i = 0; i < res; i++)
                    {
                        cout << " " << buf[i];
                    }
                    cout << endl;
                }
                */

                for (int i = 0; i < res; i++)
                {
                    char b = (char)buf[i];

                    if (b == 0) continue;

                    if (start)
                    {
                        if ((b != '\r') && (b != '\n'))
                        {
                            start = false;
                            log.append(b);
                        }
                    }
                    else
                    {
                        if ((b != '\r') && (b != '\n'))
                        {
                            log.append(b);
                        }
                        else
                        {
                            start = true;
                            log.append('\0');
                            cout << QString::fromLocal8Bit(log) << endl;
                            log.clear();
                        }
                    }
                }
            }
            else
            {
                qDebug("res: %d", res);
            }

//            QThread::msleep(500);
//            qDebug("test");
        }

        hid_close(handle);
    }
    else
    {
        qDebug("Not found");
    }

    res = hid_exit();
    if (res != 0) { qDebug("res: %d", res); }

    return 0;
}
