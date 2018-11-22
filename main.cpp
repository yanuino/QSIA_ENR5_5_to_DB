#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

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
    QString ID;
    QString Activity;
    QString Name;
    QString DIST_VER_UPPER;

    QString GEO_LAT;
    QString GEO_LONG;
    QString DIST_VER_LOWER;


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
            deb = fin = 0;
            deb = ligne1.indexOf("TXT_NAME", deb);
            deb = ligne1.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne1.indexOf("</span", deb);
            ID = ligne1.mid(deb, fin - deb);

            deb = ligne1.indexOf("<span", deb);
            deb = ligne1.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne1.indexOf("</span", deb);
            Activity = ligne1.mid(deb, fin - deb);

            deb = ligne1.indexOf("NOM_USUEL", deb);
            deb = ligne1.indexOf(">", deb);
            fin = ligne1.indexOf("</span", deb);
            deb = deb + 1;
            Name = ligne1.mid(deb, fin - deb);

            deb = ligne1.indexOf("UOM_DIST_VER_UPPER", deb);
            deb = ligne1.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne1.indexOf("</span", deb);
            DIST_VER_UPPER = ligne1.mid(deb, fin - deb);

            deb = fin = 0;
            deb = ligne2.indexOf("GEO_LAT", deb);
            deb = ligne2.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne2.indexOf("</span", deb);
            GEO_LAT = ligne2.mid(deb, fin - deb);

            deb = ligne2.indexOf("GEO_LONG", deb);
            deb = ligne2.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne2.indexOf("</span", deb);
            GEO_LONG = ligne2.mid(deb, fin - deb);

            deb = ligne2.indexOf("CODE_DIST_VER_LOWER", deb);
            deb = ligne2.indexOf(">", deb);
            deb = deb + 1;
            fin = ligne2.indexOf("</span", deb);
            DIST_VER_LOWER = ligne2.mid(deb, fin - deb);

            out << ID << ','
                << Activity << ','
                << Name << ','
                << GEO_LAT << ','
                << GEO_LONG << ','
                << DIST_VER_LOWER << ','
                << DIST_VER_UPPER
                << endl;

            // JUMP
            //l1 = eol2;
        }
        else
        {
            // WTF!
            out << "Error!" << endl;
        }
    }

    /*
     * <tr id="mid--1562507-1569074--CODE_DIST_VER_UPPER-TXT_NAME-VAL_DIST_VER_UPPER-UOM_DIST_VER_UPPER-TXT_RMK_WORK_HR-NOM_USUEL"  ><td>
                <span id="gaixm--1562507--16388575--AIRSPACE.TXT_NAME" >200</span>
              </td><td>
                <span >parachutage</span>
                <span id="gaixm--1569074--7650653--AIRSPACE_BORDER.NOM_USUEL" >LILLE MARCQ EN BAROEUL Aérodrome (59)</span>
              </td><td>
                <br />
                <span id="gaixm--1562507--16388575--AIRSPACE.VAL_DIST_VER_UPPER-AIRSPACE.CODE_DIST_VER_UPPER-AIRSPACE.UOM_DIST_VER_UPPER" >FL 140</span>
              </td><td>
                <span id="gaixm--1562507--16388575--AIRSPACE.TXT_RMK_WORK_HR" >SR-SS+30.</span>
              </td></tr><tr id="mid--1562507-1569074--GEO_LAT-VAL_DIST_VER_LOWER-UOM_DIST_VER_LOWER-GEO_LONG-TXT_RMK_NAT-CODE_DIST_VER_LOWER--1"  ><td colspan="2"   >
                <span id="gaixm--1569074--7650653--AIRSPACE_VERTEX.GEO_LAT" >50°41'17"N</span>
                ,
                <span id="gaixm--1569074--7650653--AIRSPACE_VERTEX.GEO_LONG" >003°04'36"E</span>
              </td><td>
                <span id="gaixm--1562507--16388575--AIRSPACE.VAL_DIST_VER_LOWER-AIRSPACE.UOM_DIST_VER_LOWER-AIRSPACE.CODE_DIST_VER_LOWER" >SFC</span>
              </td><td>
                <span />
                <span id="gaixm--1562507--16388575--AIRSPACE.TXT_RMK_NAT" >
                  Information des usagers sur A/A MARCQ et LILLE APP/FIS.
                  <br />
                  Activité réservée aux usagers signataires du protocole.
                  <br />
                  Users' information on A/A MARCQ and LILLE APP/FIS.
                  <br />
                  Activity reserved for signatory users to the protocol.
                </span>
              </td></tr>
     *
     *
     */



    outfile.write(outData);

    return 0;
    //return a.exec();
}
