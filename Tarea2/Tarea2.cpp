// Tarea2.cpp : Implementación de conexión ODBC a SQL Server

#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <string>
#include <iomanip>

int main()
{
    SQLHENV henv;   // Ambiente
    SQLHDBC hdbc;   // Conexión
    SQLHSTMT hstmt; // Statement
    SQLRETURN ret;  // Código de retorno

    // Inicializar el ambiente
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Error al crear el ambiente ODBC\n";
        return 1;
    }

    // Establecer la versión de ODBC
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);

    // Asignar el handle de conexión
    ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        std::cout << "Error al crear el handle de conexión\n";
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    // Conectar al DSN con autenticación de Windows y base de datos específica
    // Verificar si el driver está disponible
    SQLWCHAR driverList[1024];
    SQLSMALLINT driverListLen;
    SQLWCHAR* currentDriver = driverList;
    bool driverFound = false;

    ret = SQLDriversW(henv, SQL_FETCH_FIRST, driverList, sizeof(driverList)/sizeof(SQLWCHAR),
                     &driverListLen, NULL, 0, NULL);
    while (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        if (wcsstr(currentDriver, L"SQL Server") != NULL) {
            driverFound = true;
            break;
        }
        ret = SQLDriversW(henv, SQL_FETCH_NEXT, driverList, sizeof(driverList)/sizeof(SQLWCHAR),
                         &driverListLen, NULL, 0, NULL);
    }

    if (!driverFound) {
        std::cout << "Error: No se encontró el driver de SQL Server\n";
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    SQLWCHAR connStr[] = L"Driver={SQL Server};Server=THP11-AAGARCIA;Database=Empleados;Trusted_Connection=Yes;Connection Timeout=30;";
    std::wcout << L"Intentando conectar con: " << connStr << L"\n";
    ret = SQLDriverConnectW(hdbc, NULL, connStr, SQL_NTS,
                          NULL, 0, NULL, SQL_DRIVER_NOPROMPT);

    if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO) {
        SQLWCHAR sqlstate[6];
        SQLWCHAR message[SQL_MAX_MESSAGE_LENGTH];
        SQLINTEGER native_error;
        SQLSMALLINT message_len;
        SQLSMALLINT i = 1;
        
        while (SQLGetDiagRecW(SQL_HANDLE_DBC, hdbc, i, sqlstate, &native_error,
                            message, sizeof(message), &message_len) != SQL_NO_DATA) {
            std::wcout << L"[" << sqlstate << L"] ";
            std::wcout << message << L" (" << native_error << L")\n";
            i++;
        }
        
        std::cout << "Error al conectar con el servidor\n";
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
        return 1;
    }

    std::cout << "Conexión exitosa a la base de datos\n\n";

    // Crear y ejecutar la consulta
    ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

    // 1. Consulta de Empleados
    std::cout << "TABLA EMPLEADOS\n";
    std::cout << "+---------------+----------------------------------------+----------------------+\n";
    std::cout << "| No. Empleado  | Nombre                                   | Puesto               |\n";
    std::cout << "+---------------+----------------------------------------+----------------------+\n";

    SQLWCHAR sqlQuery[] = L"SELECT NumeroEmpleado, Nombre, Puesto FROM Empleados";

    ret = SQLExecDirectW(hstmt, sqlQuery, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLINTEGER numEmpleado;
        SQLCHAR nombre[100], puesto[50];
        SQLLEN cbNumEmpleado = 0, cbNombre = 0, cbPuesto = 0;

        SQLBindCol(hstmt, 1, SQL_C_LONG, &numEmpleado, 0, &cbNumEmpleado);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, nombre, sizeof(nombre), &cbNombre);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, puesto, sizeof(puesto), &cbPuesto);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "| " << std::setw(14) << std::left << numEmpleado
                      << "| " << std::setw(39) << std::left << std::string((char*)nombre)
                      << "| " << std::setw(19) << std::left << std::string((char*)puesto) << " |" << std::endl;
        }
    } else {
        std::cout << "Error al ejecutar la consulta SQL de Empleados\n";
    }
    std::cout << "+---------------+----------------------------------------+----------------------+\n\n";
    SQLFreeStmt(hstmt, SQL_CLOSE);

    // 2. Consulta de Centros de Trabajo
    std::cout << "TABLA CENTROS DE TRABAJO\n";
    std::cout << "+---------------+----------------------------------------+----------------------+\n";
    std::cout << "| No. Centro    | Nombre Centro                            | Ciudad               |\n";
    std::cout << "+---------------+----------------------------------------+----------------------+\n";

    SQLWCHAR sqlQueryCentros[] = L"SELECT NumeroCentro, NombreCentro, Ciudad FROM CentrosTrabajo";
    ret = SQLExecDirectW(hstmt, sqlQueryCentros, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {   
        SQLINTEGER numeroCentro;
        SQLCHAR nombreCentro[100], ciudad[50];
        SQLLEN cbNumeroCentro = 0, cbNombreCentro = 0, cbCiudad = 0;

        SQLBindCol(hstmt, 1, SQL_C_LONG, &numeroCentro, 0, &cbNumeroCentro);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, nombreCentro, sizeof(nombreCentro), &cbNombreCentro);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, ciudad, sizeof(ciudad), &cbCiudad);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "| " << std::setw(14) << std::left << numeroCentro
                      << "| " << std::setw(39) << std::left << std::string((char*)nombreCentro)
                      << "| " << std::setw(19) << std::left << std::string((char*)ciudad) << " |" << std::endl;
        }
    } else {
        std::cout << "Error al ejecutar la consulta SQL de Centros de Trabajo\n";
    }
    std::cout << "+---------------+----------------------------------------+----------------------+\n\n";
    SQLFreeStmt(hstmt, SQL_CLOSE);

    // 3. Consulta de Directivos
    std::cout << "TABLA DIRECTIVOS\n";
    std::cout << "+---------------+----------------------------------------+----------------------+\n";
    std::cout << "| ID Directivo  | No. Centro Supervisa                     | Prestación Comb.     |\n";
    std::cout << "+---------------+----------------------------------------+----------------------+\n";

    SQLWCHAR sqlQueryDirectivos[] = L"SELECT DirectivoID, NumeroCentroSupervisa, PrestacionCombustible FROM Directivos";
    ret = SQLExecDirectW(hstmt, sqlQueryDirectivos, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLINTEGER directivoID, numeroCentroSupervisa;
        SQLCHAR prestacionCombustible[2];
        SQLLEN cbDirectivoID = 0, cbNumeroCentroSupervisa = 0, cbPrestacionCombustible = 0;

        SQLBindCol(hstmt, 1, SQL_C_LONG, &directivoID, 0, &cbDirectivoID);
        SQLBindCol(hstmt, 2, SQL_C_LONG, &numeroCentroSupervisa, 0, &cbNumeroCentroSupervisa);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, prestacionCombustible, sizeof(prestacionCombustible), &cbPrestacionCombustible);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "| " << std::setw(14) << std::left << directivoID
                      << "| " << std::setw(39) << std::left << numeroCentroSupervisa
                      << "| " << std::setw(19) << std::left << (prestacionCombustible[0] == '1' ? "Sí" : "No") << " |" << std::endl;
        }
    } else {
        std::cout << "Error al ejecutar la consulta SQL de Directivos\n";
    }
    std::cout << "+---------------+----------------------------------------+----------------------+\n";
    SQLFreeStmt(hstmt, SQL_CLOSE);


    // 4. Consulta final de Empleados detallados
    std::cout << "\nTABLA EMPLEADOS DETALLES\n";
    std::cout << "+-------------------+------------------------------+---------------------+--------------------+------------------------------+----------------------+---------------+\n";
    std::cout << "| Numero Empleado  | Nombre Completo              | Fecha Nacimiento   | RFC                | Nombre de su Centro          | Puesto               | ¿Es directivo?|\n";
    std::cout << "+-------------------+------------------------------+---------------------+--------------------+------------------------------+----------------------+---------------+\n";

    SQLWCHAR sqlQueryFinal[] = 
           L"SELECT e.NumeroEmpleado, "
           L"(e.Nombre + ' ' + e.ApellidoPaterno + ' ' + e.ApellidoMaterno) AS NombreCompleto, "
           L"e.FechaNacimiento, e.RFC, c.NombreCentro, e.Puesto, "
           L"CASE WHEN d.DirectivoID IS NOT NULL THEN 1 ELSE 0 END AS EsDirectivo "
           L"FROM Empleados e "
           L"LEFT JOIN CentrosTrabajo c ON e.CentroTrabajoID = c.NumeroCentro "
           L"LEFT JOIN Directivos d ON e.NumeroEmpleado = d.DirectivoID";

    ret = SQLExecDirectW(hstmt, sqlQueryFinal, SQL_NTS);
    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
        SQLINTEGER numEmpleado, esDirectivo;
        SQLCHAR nombreCompleto[200], fechaNacimiento[20], rfc[20], nombreCentro[100], puesto[50];
        SQLLEN cbNumEmpleado = 0, cbNombreCompleto = 0, cbFechaNacimiento = 0, cbRFC = 0, cbNombreCentro = 0, cbPuesto = 0, cbEsDirectivo = 0;

        SQLBindCol(hstmt, 1, SQL_C_LONG, &numEmpleado, 0, &cbNumEmpleado);
        SQLBindCol(hstmt, 2, SQL_C_CHAR, nombreCompleto, sizeof(nombreCompleto), &cbNombreCompleto);
        SQLBindCol(hstmt, 3, SQL_C_CHAR, fechaNacimiento, sizeof(fechaNacimiento), &cbFechaNacimiento);
        SQLBindCol(hstmt, 4, SQL_C_CHAR, rfc, sizeof(rfc), &cbRFC);
        SQLBindCol(hstmt, 5, SQL_C_CHAR, nombreCentro, sizeof(nombreCentro), &cbNombreCentro);
        SQLBindCol(hstmt, 6, SQL_C_CHAR, puesto, sizeof(puesto), &cbPuesto);
        SQLBindCol(hstmt, 7, SQL_C_LONG, &esDirectivo, 0, &cbEsDirectivo);

        while (SQLFetch(hstmt) == SQL_SUCCESS) {
            std::cout << "| " << std::setw(17) << std::left << numEmpleado
                      << "| " << std::setw(28) << std::left << std::string((char*)nombreCompleto)
                      << "| " << std::setw(21) << std::left << std::string((char*)fechaNacimiento)
                      << "| " << std::setw(18) << std::left << std::string((char*)rfc)
                      << "| " << std::setw(28) << std::left << std::string((char*)nombreCentro)
                      << "| " << std::setw(22) << std::left << std::string((char*)puesto)
                      << "| " << std::setw(13) << std::left << (esDirectivo == 1 ? "Si" : "No") << "|" << std::endl;
        }
    } else {
        std::cout << "Error al ejecutar la consulta SQL detallada de empleados\n";
    }
    std::cout << "+-------------------+------------------------------+---------------------+--------------------+------------------------------+----------------------+---------------+\n";
    SQLFreeStmt(hstmt, SQL_CLOSE);

    // Liberar el statement handle
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

    // Limpiar los handles
    SQLDisconnect(hdbc);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);

    return 0;
}


