import os
import unittest
import gzip
import struct

import pb
import deviceapps_pb2


MAGIC = 0xFFFFFFFF
DEVICE_APPS_TYPE = 1
TEST_FILE = "test.pb.gz"
HEADER_SIZE = 8

class TestPB(unittest.TestCase):
    deviceapps = [
        {"device": {"type": "idfa", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7c"},
         "lat": 67.7835424444, "lon": -22.8044005471, "apps": [1, 2, 3, 4]},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "lat": 42, "lon": -42, "apps": [1, 2]},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "lat": 42, "lon": -42, "apps": []},
        {"device": {"type": "gaid", "id": "e7e1a50c0ec2747ca56cd9e1558c0d7d"}, "apps": [1]},
    ]

    def tearDown(self):
        os.remove(TEST_FILE)

    def test_write(self):
        bytes_written = pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        self.assertTrue(bytes_written > 0)
        # check magic, type, etc.
        with gzip.open(TEST_FILE) as zfile:
            for deviceapp_orig in self.deviceapps:
                header = zfile.read(HEADER_SIZE)
                magic, device_apps_type, length = struct.unpack('<IHH', header)
                self.assertEqual((MAGIC, DEVICE_APPS_TYPE), (magic, device_apps_type))                

                body = zfile.read(length)
                deviceapp_subj = deviceapps_pb2.DeviceApps()
                deviceapp_subj.ParseFromString(body)
                self.assertEqual(deviceapp_subj.device.type, deviceapp_orig.get('device', {}).get('type', '').encode())
                self.assertEqual(deviceapp_subj.device.id, deviceapp_orig.get('device', {}).get('id', '').encode())
                self.assertEqual(deviceapp_subj.apps, deviceapp_orig.get('apps', []))
                self.assertEqual(deviceapp_subj.lat, deviceapp_orig.get('lat', 0))
                self.assertEqual(deviceapp_subj.lon, deviceapp_orig.get('lon', 0))


#    @unittest.skip("Optional problem")
    def test_read(self):
        pb.deviceapps_xwrite_pb(self.deviceapps, TEST_FILE)
        for i, d in enumerate(pb.deviceapps_xread_pb(TEST_FILE)):
            self.assertEqual(d, self.deviceapps[i])
