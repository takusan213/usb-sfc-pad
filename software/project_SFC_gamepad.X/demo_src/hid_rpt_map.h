#ifndef HID_RPT_MAP_H
#define HID_RPT_MAP_H

#define HID_MAP_RPT_DESC_SIZE 21   // レポートディスクリプタのサイズ
#define HID_MAP_EP_BUF_SIZE   64   // USB EP送受信バッファのサイズ

const struct{uint8_t report[HID_MAP_RPT_DESC_SIZE];}hid_map_rpt={{ 
  0x06,0x00,0xFF,            // Usage Page (Vendor Defined Page 1, 0xFF00)
  0x09,0x01,                 // Usage (Vendor Usage 1)
  0xA1,0x01,                 // Collection (Application)
  0x15,0x00,                 //   Logical Minimum (0)
  0x26,0xFF,0x00,           //   Logical Maximum (255)
  0x75,0x08,                 //   Report Size (8)
  0x95,0x40,                 //   Report Count (64) - for 64 bytes total including Report ID
  0x09,0x01,                 //   Usage (Vendor Usage 1)
  0xB1,0x02,                 //   Feature (Data, Variable, Absolute)
  0xC0                       //   End Collection
}};

#endif // HID_RPT_MAP_H