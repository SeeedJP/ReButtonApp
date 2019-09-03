#include "sastoken.h"
#include <stdlib.h>
#include <string.h>
#include <mbedtls/base64.h>
#include <mbedtls/md.h>
#include <mbedtls/md_internal.h>

// Copy from https://github.com/MXCHIP/IoTDevKit/blob/master/pnp/iotc_devkit/Device/src/mxchip_iot_devkit/ui/setting_page.c

static int base64DecodeKey(const unsigned char* input, int input_len, unsigned char** output, int* output_len)
{
	unsigned char* buffer = NULL;
	size_t len = 0;
	if (mbedtls_base64_decode(NULL, 0, &len, input, (size_t)input_len) == MBEDTLS_ERR_BASE64_INVALID_CHARACTER)
	{
		return -1;
	}
	buffer = (unsigned char*)calloc(1, len);
	if (buffer == NULL)
	{
		return -2;
	}
	if (mbedtls_base64_decode(buffer, len, &len, input, (size_t)input_len))
	{
		free(buffer);
		return -3;
	}
	*output = buffer;
	*output_len = len;
	return 0;
}

static int base64EncodeKey(const unsigned char* input, int input_len, char** output)
{
	size_t len = 0;
	unsigned char* buffer = NULL;
	mbedtls_base64_encode(NULL, 0, &len, input, (size_t)input_len);
	if (len == 0)
	{
		return -1;
	}
	buffer = (unsigned char*)calloc(1, len + 1);
	if (buffer == NULL)
	{
		return -2;
	}
	if (mbedtls_base64_encode(buffer, len, &len, input, (size_t)input_len))
	{
		free(buffer);
		return -3;
	}
	*output = (char*)buffer;
	return 0;
}

int GenerateDeviceSasToken(const char* pkey, const char* device_id, char** sas_token)
{
	// Decoode key
	unsigned char* key_data = NULL;
	int key_len = 0;
	if (base64DecodeKey((const unsigned char*)pkey, strlen(pkey), &key_data, &key_len))
	{
		return 1;
	}

	char* token = NULL;
	unsigned char token_data[MBEDTLS_MD_MAX_SIZE] = { 0 };
	mbedtls_md_context_t ctx;
	mbedtls_md_init(&ctx);
	if (mbedtls_md_setup(&ctx, &mbedtls_sha256_info, 1))
	{
		goto _exit;
	}
	if (mbedtls_md_hmac_starts(&ctx, key_data, key_len))
	{
		goto _exit;
	}
	if (mbedtls_md_hmac_update(&ctx, (const unsigned char*)device_id, (size_t)strlen(device_id)))
	{
		goto _exit;
	}
	if (mbedtls_md_hmac_finish(&ctx, token_data))
	{
		goto _exit;
	}
	if (base64EncodeKey(token_data, mbedtls_sha256_info.size, &token))
	{
		goto _exit;
	}
	*sas_token = token;

_exit:
	if (key_data)
	{
		free(key_data);
	}
	mbedtls_md_free(&ctx);
	return (token ? 0 : -1);
}
