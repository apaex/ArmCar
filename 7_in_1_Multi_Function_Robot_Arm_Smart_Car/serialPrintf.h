#pragma once

/*
 * SerialPrintf
 * Реализует функциональность printf в Serial.print
 * Применяется для отладочной печати
 * Параметры как у printf
 * Возвращает 
 *		0 - ошибка формата
 *		отрицательное чило - нехватка памяти, модуль числа равен запрашиваемой памяти
 *		положительное число - количество символов, выведенное в Serial
 */
const size_t SerialPrintf (const char *szFormat, ...) {
	va_list argptr;
	va_start(argptr, szFormat);
	char *szBuffer = 0;
	const size_t nBufferLength = vsnprintf(szBuffer, 0, szFormat, argptr) + 1;
	if (nBufferLength == 1) return 0;
	szBuffer = (char *) malloc(nBufferLength);
	if (! szBuffer) return - nBufferLength;
	vsnprintf(szBuffer, nBufferLength, szFormat, argptr);
	Serial.print(szBuffer);
	free(szBuffer);
	return nBufferLength - 1;
}

