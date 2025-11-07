#include "../include/network.h"

#ifdef __linux__
// Writes a value to a Bluetooth GATT characteristic using D-Bus.
// conn: D-Bus connection.
// char_path: D-Bus object path of the characteristic.
// message: Data to write (as a string).
int write_characteristic(DBusConnection *conn, const char *char_path, const char *message)
{
	DBusMessage *msg;
	DBusMessageIter iter, array_iter;
	uint8_t *data = (uint8_t *)message;
	int len = strlen(message);

	// Create a new method call message for WriteValue
	msg = dbus_message_new_method_call(
		"org.bluez",
		char_path,
		"org.bluez.GattCharacteristic1",
		"WriteValue"
	);

	if (!msg)
		return -1;

	dbus_message_iter_init_append(msg, &iter);

	// Append the byte array (the message data)
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "y", &array_iter);
	dbus_message_iter_append_fixed_array(&array_iter, DBUS_TYPE_BYTE, &data, len);
	dbus_message_iter_close_container(&iter, &array_iter);

	// Append empty options dictionary
	DBusMessageIter dict;
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);
	dbus_message_iter_close_container(&iter, &dict);

	// Send the message and wait for a reply
	DBusMessage *reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, NULL);
	dbus_message_unref(msg);

	if (!reply)
		return -1;

	dbus_message_unref(reply);
	return 0;
}
#endif
