#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QtSql>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QFile file("test.html");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        exit(1);

    QFile outfile("out.csv");
    if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text))
        exit(2);

    QTextStream in(&file);

    QByteArray tempData;
    QTextStream temp(&tempData, QIODevice::ReadWrite);

    QByteArray outData;
    QTextStream out(&outData, QIODevice::ReadWrite);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("enr5_5.db");

    if (!db.open())
        //return db.lastError();
        exit(1000);

    QStringList tables = db.tables();
    if (!tables.contains("enr", Qt::CaseInsensitive))
    {
        QSqlQuery q;
        if (!q.exec(QLatin1String("create table enr(id varchar unique not null, activity varchar, name varchar, geo_lat real, geo_long real, dist_ver_upper varchar, dist_ver_max varchar, dist_ver_lower varchar)")))
            //return q.lastError();
            exit(1001);
    }

    // CLEANNING

    while (!in.atEnd())
    {
        QString line = in.readLine();
        line.remove("Visibility=\"None\"");
        line.remove("Page-break=\"None\"");
        line.remove("rowspan=\"1\"");
        line.remove("colspan=\"1\"");

        temp << line << endl;
    }

    int deb = 0, fin = 0 ;
    while ( (deb = tempData.indexOf("<thead", deb)) >= 0 )
    {
        deb = deb + 6;
        fin = tempData.indexOf(">", deb);
        if ( fin != -1 )
            tempData.remove(deb, fin - deb);
    }

    deb = fin = 0;
    while ( (deb = tempData.indexOf("<th", deb)) >= 0 )
    {
        deb = deb + 3;
        fin = tempData.indexOf(">", deb);
        if (tempData.mid(deb, fin -deb).contains("colspan"))
        {
            deb = deb +1;
            fin = tempData.indexOf("colspan", deb);
        }

        if ( fin != -1 )
            tempData.remove(deb, fin - deb);
    }

    deb = fin = 0;
    while ( (deb = tempData.indexOf("<td", deb)) >= 0 )
    {
        deb = deb + 3;
        fin = tempData.indexOf(">", deb);
        if (tempData.mid(deb, fin - deb).contains("colspan"))
        {
            deb = deb +1;
            fin = tempData.indexOf("colspan", deb);
        }
        if ( fin != -1 )
            tempData.remove(deb, fin - deb);
    }

    deb = fin = 0;
    while ( (deb = tempData.indexOf("<del", deb)) >= 0 )
    {
        fin = tempData.indexOf("/del>", deb);
        if ( fin != -1 )
            tempData.remove(deb, (fin - deb) + 5);
    }

    deb = fin = 0;
    while ( (deb = tempData.indexOf("</ins>", deb)) >= 0 )
    {
            tempData.remove(deb, 6);
    }

    deb = fin = 0;
    while ( (deb = tempData.indexOf("<ins", deb)) >= 0 )
    {
        fin = tempData.indexOf(">", deb);
        if ( fin != -1 )
            tempData.remove(deb, fin + 1 - deb);
    }

    // GET RECORDS
    /*
     * Un enregistrement = 2 lignes
     * Début de ligne 1:
     * - <tr ... CODE_DIST_VER_UPPER-TXT_NAME-VAL_DIST_VER_UPPER-UOM_DIST_VER_UPPER-TXT_RMK_WORK_HR-NOM_USUEL
     * Fin de ligne 1:
     * - </tr>
     * Début de ligne 2:
     * - <tr ... GEO_LAT-VAL_DIST_VER_LOWER-UOM_DIST_VER_LOWER-GEO_LONG-TXT_RMK_NAT-CODE_DIST_VER_LOWER--1
     * Fin de ligne 2:
     * - </tr>
     */
    QString Id;
    QString Activity;
    QString Name;
    QString Dist_Ver_Upper;
    QString Dist_Ver_Max;
    QString Geo_Lat;
    QString Geo_Long;
    QString Dist_Ver_Lower;


    QString ligne1, ligne2;
    int l1 = 0, eol1 = 0;
    int l2 = 0, eol2 = 0;


    // Correction des truc chiants:

    tempData.replace("GEO_LAT-VAL_DIST_VER_LOWER-UOM_DIST_VER_LOWER-GEO_LONG-CODE_DIST_VER_LOWER-TXT_RMK_NAT--1",
                     "GEO_LAT-VAL_DIST_VER_LOWER-UOM_DIST_VER_LOWER-GEO_LONG-TXT_RMK_NAT-CODE_DIST_VER_LOWER--1");

    tempData.replace("UOM_DIST_VER_MAX-CODE_DIST_VER_UPPER-CODE_DIST_VER_MAX-TXT_NAME-VAL_DIST_VER_UPPER-UOM_DIST_VER_UPPER-TXT_RMK_WORK_HR-VAL_DIST_VER_MAX-NOM_USUEL",
                     "CODE_DIST_VER_UPPER-TXT_NAME-VAL_DIST_VER_UPPER-UOM_DIST_VER_UPPER-TXT_RMK_WORK_HR-NOM_USUEL");

    QFile tempfile("temp.html");
    if (!tempfile.open(QIODevice::WriteOnly | QIODevice::Text))
        exit(2);
    tempfile.write(tempData);

    while ( (l1 = tempData.indexOf("CODE_DIST_VER_UPPER-TXT_NAME-VAL_DIST_VER_UPPER-UOM_DIST_VER_UPPER-TXT_RMK_WORK_HR-NOM_USUEL", l1)) >= 0)
    {
        Id = "";
        Activity = "";
        Name = "";
        Dist_Ver_Upper = "";
        Dist_Ver_Max = "";

        Geo_Lat = "";
        Geo_Long = "";
        Dist_Ver_Lower = "";

        l1 = l1 + QString("CODE_DIST_VER_UPPER-TXT_NAME-VAL_DIST_VER_UPPER-UOM_DIST_VER_UPPER-TXT_RMK_WORK_HR-NOM_USUEL").size();

        eol1 = tempData.indexOf("</tr>", l1);

        ligne1 = tempData.mid(l1 , eol1 - l1 ).simplified();

        l2 = tempData.indexOf("GEO_LAT-VAL_DIST_VER_LOWER-UOM_DIST_VER_LOWER-GEO_LONG-TXT_RMK_NAT-CODE_DIST_VER_LOWER--1", l1);
        if (l2 >= 0)
        {
            l2 = l2 + QString("GEO_LAT-VAL_DIST_VER_LOWER-UOM_DIST_VER_LOWER-GEO_LONG-TXT_RMK_NAT-CODE_DIST_VER_LOWER--1").size();

            eol2 = tempData.indexOf("</tr>", l2);

            ligne2 = tempData.mid(l2 , eol2 - l2 ).simplified();

            // DECODE
            int borne = -1;
            deb = fin = 0;
            deb = ligne1.indexOf("TXT_NAME", deb);
            deb = ligne1.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne1.indexOf("</span", deb);
            Id = ligne1.mid(deb, fin - deb).simplified();

            deb = ligne1.indexOf("<span", deb);
            deb = ligne1.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne1.indexOf("</span", deb);
            Activity = ligne1.mid(deb, fin - deb).simplified();

            deb = ligne1.indexOf("NOM_USUEL", deb);
            deb = ligne1.indexOf(">", deb);
            borne = ligne1.indexOf("</td>", deb);
            fin = ligne1.indexOf("</span", deb);
            if ( fin < borne)
            {
                deb = deb + 1;
                Name = ligne1.mid(deb, fin - deb).simplified();
            }

            deb = ligne1.indexOf("UOM_DIST_VER_UPPER", deb);
            deb = ligne1.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne1.indexOf("</span", deb);
            Dist_Ver_Upper = ligne1.mid(deb, fin - deb).simplified();

            int debmax = ligne1.indexOf("CODE_DIST_VER_MAX", deb);
            if (debmax > -1)
            {
                deb = ligne1.indexOf(">", debmax);
                deb = deb + 1;
                fin = ligne1.indexOf("</span", deb);
                Dist_Ver_Max = ligne1.mid(deb, fin - deb).simplified();
            }

            deb = fin = 0;
            deb = ligne2.indexOf("GEO_LAT", deb);
            deb = ligne2.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne2.indexOf("</span", deb);
            Geo_Lat = ligne2.mid(deb, fin - deb).simplified();

            deb = ligne2.indexOf("GEO_LONG", deb);
            deb = ligne2.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne2.indexOf("</span", deb);
            Geo_Long = ligne2.mid(deb, fin - deb).simplified();

            deb = ligne2.indexOf("CODE_DIST_VER_LOWER", deb);
            deb = ligne2.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne2.indexOf("</span", deb);
            Dist_Ver_Lower = ligne2.mid(deb, fin - deb).simplified();

            // CONVERT
            float geo_lat_h, geo_lat_m, geo_lat_s, geo_lat = 0.0f;
            QRegExp rxlat("(^\\d+)°(\\d+)'(\\d+)\"([NS])$");
            int poslat = rxlat.indexIn(Geo_Lat);
            if (poslat > -1) {
                geo_lat_h = rxlat.cap(1).toFloat();
                geo_lat_m = rxlat.cap(2).toFloat();
                geo_lat_s = rxlat.cap(3).toFloat();
                geo_lat = geo_lat_h + geo_lat_m / 60.0f + geo_lat_s / 3600.0f;
                int dummy= static_cast<int>(geo_lat*100000.0f);
                geo_lat = static_cast<float>(dummy/100000.0f);
            }

            float geo_long_h, geo_long_m, geo_long_s, geo_long = 0.0f;
            QRegExp rxlong("(^\\d+)°(\\d+)'(\\d+)\"([EW])$");
            int poslong = rxlong.indexIn(Geo_Long);
            if (poslong > -1) {
                geo_long_h = rxlong.cap(1).toFloat();
                geo_long_m = rxlong.cap(2).toFloat();
                geo_long_s = rxlong.cap(3).toFloat();
                geo_long = geo_long_h + geo_long_m / 60.0f + geo_long_s / 3600.0f;
                int dummy= static_cast<int>(geo_long*100000.0f);
                geo_long = static_cast<float>(dummy/100000.0f);
            }

            out.setRealNumberPrecision(8);
            out << Id << ','
                << Activity << ','
                << Name << ','
                << Geo_Lat << ',' << geo_lat << ','
                << Geo_Long << ',' << geo_long << ','
                << Dist_Ver_Lower << ','
                << Dist_Ver_Upper
                << endl;
            if ( true ) //( Activity == "aéromodélisme")
            {
                QSqlQuery qins;
                qins.prepare("INSERT INTO enr (id , activity , name , geo_lat , geo_long , dist_ver_upper, dist_ver_max , dist_ver_lower)"
                             "VALUES (:id , :activity , :name , :geo_lat , :geo_long , :dist_ver_upper, :dist_ver_max , :dist_ver_lower)");
                qins.bindValue(":id", Id);
                qins.bindValue(":activity", Activity);
                qins.bindValue(":name", Name);
                qins.bindValue(":geo_lat", geo_lat);
                qins.bindValue(":geo_long", geo_long);
                qins.bindValue(":dist_ver_lower", Dist_Ver_Lower);
                qins.bindValue(":dist_ver_max", Dist_Ver_Max);
                qins.bindValue(":dist_ver_upper", Dist_Ver_Upper);
                qins.exec();
            }
            // JUMP
            //l1 = eol2;
        }
        else
        {
            // WTF!
            out << "Error!" << endl;
        }
    }

    outfile.write(outData);

    return 0;
    //return a.exec();
}
