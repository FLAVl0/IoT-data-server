#include <dbus/dbus.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Fonction pour écrire sur la caractéristique
int write_characteristic(DBusConnection* conn, const char* char_path, const char* message) {
	DBusMessage* msg;
	DBusMessageIter iter, array_iter;
	uint8_t* data = (uint8_t*)message;
	int len = strlen(message);

	msg = dbus_message_new_method_call(
		"org.bluez",
		char_path,
		"org.bluez.GattCharacteristic1",
		"WriteValue"
	);
	if (!msg) return -1;

	dbus_message_iter_init_append(msg, &iter);

	// Tableau d'octets
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "y", &array_iter);
	dbus_message_iter_append_fixed_array(&array_iter, DBUS_TYPE_BYTE, &data, len);
	dbus_message_iter_close_container(&iter, &array_iter);

	// Options vides
	DBusMessageIter dict;
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict);
	dbus_message_iter_close_container(&iter, &dict);

	DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, NULL);
	dbus_message_unref(msg);

	if (!reply) return -1;
	dbus_message_unref(reply);
	return 0;
}

// Fonction pour activer les notifications
int start_notify(DBusConnection* conn, const char* char_path) {
	DBusMessage* msg = dbus_message_new_method_call(
		"org.bluez",
		char_path,
		"org.bluez.GattCharacteristic1",
		"StartNotify"
	);
	if (!msg) return -1;

	DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, NULL);
	dbus_message_unref(msg);

	if (!reply) return -1;
	dbus_message_unref(reply);
	printf("✅ Notifications activées sur %s\n", char_path);
	return 0;
}

// Fonction pour écouter les notifications
void listen_notifications(DBusConnection* conn) {
	while (1) {
		dbus_connection_read_write(conn, 100);
		DBusMessage* msg = dbus_connection_pop_message(conn);

		if (!msg) {
			usleep(100000);
			continue;
		}

		if (dbus_message_is_signal(msg, "org.freedesktop.DBus.Properties", "PropertiesChanged")) {
			DBusMessageIter iter, dict, entry, variant, array;
			dbus_message_iter_init(msg, &iter);
			dbus_message_iter_next(&iter); // passer l'interface
			dbus_message_iter_recurse(&iter, &dict);

			while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
				dbus_message_iter_recurse(&dict, &entry);
				const char* key;
				dbus_message_iter_get_basic(&entry, &key);

				if (strcmp(key, "Value") == 0) {
					dbus_message_iter_next(&entry);
					dbus_message_iter_recurse(&entry, &variant);
					dbus_message_iter_recurse(&variant, &array);

					unsigned char* bytes;
					int len;
					dbus_message_iter_get_fixed_array(&array, &bytes, &len);

					printf("Notification [%d bytes]: ", len);
					for (int i = 0; i < len; i++) printf("%c", bytes[i]);
					printf("\n");
				}

				dbus_message_iter_next(&dict);
			}
		}

		dbus_message_unref(msg);
	}
}

int main(int argc, char** argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <char_path> <message>\n", argv[0]);
		fprintf(stderr, "Exemple : %s /org/bluez/hci0/dev_XX_XX_XX_XX_XX_XX/service0011/char0012 TEMP\n", argv[0]);
		return 1;
	}

	const char* char_path = argv[1];   // chemin complet de la caractéristique
	const char* message = argv[2];     // commande à envoyer (ON, OFF, STATE, TEMP, DATA)

	DBusError err;
	dbus_error_init(&err);
	DBusConnection* conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (!conn) {
		fprintf(stderr, "Erreur DBus: %s\n", err.message);
		return 1;
	}

	// Activer notifications
	if (start_notify(conn, char_path) != 0) {
		fprintf(stderr, "Impossible d’activer les notifications.\n");
		return 1;
	}

	// Envoyer la commande
	if (write_characteristic(conn, char_path, message) != 0) {
		fprintf(stderr, "Erreur lors de l'écriture sur la caractéristique\n");
		return 1;
	}
	printf("Message envoyé : %s\n", message);

	// Boucle pour recevoir les notifications
	listen_notifications(conn);

	return 0;
}
