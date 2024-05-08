


# Лабораторная работа 3

**Дисциплина:** "Системы ввода/вывода"

**Цель работы:** получить знания и навыки разработки драйверов сетевых интерфейсов для операционной системы Linux.

**Название:** "Разработка драйверов сетевых устройств"

**Вариант:** 2

**Выполнили:**

- Мухаметгалеев Даниил, P33311
- Афанасьев Даниил, P33311

## Описание функциональности драйвера

    1.1. Драйвер должен создавать виртуальный сетевой интерфейс в ОС
    Linux.
    1.2. Созданный сетевой интерфейс должен перехватывать пакеты
    родительского интерфейса (eth0 или другого).
    1.3. Сетевой интерфейс должен реализовывать логику работы с
    перехваченным трафиком в соответствии с заданиями по
    вариантам.
    1.4. Должна иметься возможность просмотра статистики работы
    созданного интерфейса.

## Инструкция по сборке
Чтобы сбилдить проект
```
make build
```
Чтобы установить модуль в систему
```
make install
```
Чтобы удалить драйвер
```
make remove
```
Чтобы отчистить директорий от файлов сборки
```
make clean
```
## Инструкция пользователя

1. Собрать модуль, инструкция см. выше
2. Установить модуль в систему, см. выше


## Примеры использования

    ip a
        vni0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UNKNOWN group default qlen 1000
        link/ether e6:a5:df:b3:95:0c brd ff:ff:ff:ff:ff:ff
        inet 192.168.1.156/24 brd 192.168.1.255 scope global dynamic noprefixroute vni0
        valid_lft 3069242948sec preferred_lft 3069242948sec
        inet6 fe80::4c9c:bca1:d94a:ba07/64 scope link tentative 
        valid_lft forever preferred_lft forever


    echo "test_packet" | nc -u <ip_addr> <port>

    dmesg
        [  131.532871] lab3: no symbol version for module_layout
        [  131.548925] Module lab3 loaded
        [  131.548937] lab3: create link vni0
        [  131.548942] lab3: registered rx handler for eth0
        [  131.653860] IPv6: ADDRCONF(NETDEV_UP): vni0: link is not ready
        [  131.654178] vni0: device opened
        [  131.718161] Captured UDP datagram, saddr: 0.0.0.0
        [  131.718171] daddr: 0.0.0.0
        [  131.758183] Captured UDP datagram, saddr: 0.0.0.0
        [  131.758193] daddr: 0.0.0.0
        [  132.668152] Captured UDP datagram, saddr: 0.0.0.0
        [  132.668176] daddr: 0.0.0.0
        [  132.728190] Captured UDP datagram, saddr: 0.0.0.0
        [  132.728201] daddr: 0.0.0.0
        [  132.808205] Captured UDP datagram, saddr: 0.0.0.0
        [  132.808215] daddr: 0.0.0.0
        [  133.468189] Captured UDP datagram, saddr: 0.0.0.0
        [  133.468200] daddr: 0.0.0.0
        [  133.788154] Captured UDP datagram, saddr: 0.0.0.0
        [  133.788165] daddr: 0.0.0.0

    