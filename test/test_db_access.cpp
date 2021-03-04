#include <QtTest/QtTest>

#include "../src/db_access.h"

class TestDbAccess : public QObject
{
    Q_OBJECT

private slots:
    void testCreateTables();
};

void TestDbAccess::testCreateTables()
{
    QString str = "Hello";
    QVERIFY(str.toUpper() == "HELLO");
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

QTEST_MAIN(TestDbAccess)

#include "test_db_access.moc"