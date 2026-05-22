# Fix-ESP32-Keep-Restarting

Vietnamese

## ❓ Nếu ai đã từng nghe issue "Keep Restarting" thì nguyên nhân là gì?

Khi làm việc với ESP32, hiện tượng chip tự reset liên tục là một "cơn ác mộng" rất phổ biến. Nguyên nhân thực chất xoay quanh 3 yếu tố cốt lõi:
1. **Lỗi nghẽn CPU (Phần mềm):** Code bị rơi vào vòng lặp vô hạn hoặc xử lý tác vụ quá lâu khiến hệ thống FreeRTOS bị đóng băng.
2. **Lỗi vi phạm bộ nhớ (Phần mềm):** Ghi đè mảng, gọi con trỏ rỗng (`NULL`) làm sập nhân xử lý (Core Panic).
3. **Lỗi sụt áp nguồn (Phần cứng):** Nguồn cấp không đủ dòng điện khi chip xử lý các tác vụ nặng (như nạp chip, khởi động ngoại vi), kích hoạt cơ chế bảo vệ Brownout.

## 🚨 Chúng ta đã ra về đó là lỗi `0x10` vì...

Khi mở Serial Monitor lên, nếu bạn nhìn thấy dòng log: **`rst:0x10 (RTCWDT_RTC_RESET)`** hoặc **`Task Watchdog got triggered`**, thì "thủ phạm" chính là **Watchdog Timer (WDT) - Con chó canh cửa của hệ thống**. 

Chúng ta khẳng định đó là lỗi `0x10` vì:
* **Hệ thống bị "bỏ đói":** ESP32 chạy trên nền hệ điều hành FreeRTOS. Bộ đếm WDT được sinh ra để đảm bảo không có tác vụ nào chiếm dụng CPU mãi mãi. Nếu bạn dùng lệnh `delay()` quá lớn hoặc chạy vòng lặp `while`/`for` quá nặng mà không trả lại quyền kiểm tra cho hệ thống, WDT sẽ bị cạn năng lượng (Time-out).
* **Cơ chế tự cứu:** Khi WDT bị tràn, chip hiểu rằng phần mềm đã bị đóng băng hoàn toàn. Để tự cứu mình, ESP32 sẽ ép hệ thống phải khởi động lại ngay lập tức (Hard Reset), tạo nên vòng lặp `0x10` vô hạn.

---
English:

## ❓ If you've ever heard the "Keep Restarting" issue, what's the cause?

When working with ESP32, the phenomenon of the chip constantly resetting itself is a very common "nightmare." The actual cause revolves around three core factors:
1. **CPU bottleneck (Software):** Code gets into an infinite loop or processes tasks for too long, causing the FreeRTOS system to freeze.

2. **Memory violation (Software):** Overwriting arrays, calling null pointers (`NULL`) causes core crashes (Core Panic).

3. **Power supply voltage drop (Hardware):** Insufficient power supply when the chip is processing heavy tasks (such as chip programming, peripheral booting), triggering Brownout protection.

## 🚨 We were told that error `0x10` was due to...

When you open Serial Monitor, if you see the log line: **`rst:0x10 (RTCWDT_RTC_RESET)`** or **`Task Watchdog got triggered`**, then the culprit is the **Watchdog Timer (WDT) - the system's watchdog**.

We confirm that it is error `0x10` because:
* **System "starved":** The ESP32 runs on the FreeRTOS operating system. The WDT counter is designed to ensure that no task occupies the CPU indefinitely. If you use an excessively large `delay()` command or run a heavy `while`/`for` loop without returning the check to the system, the WDT will run out of power (Time-out).

* **Self-rescue mechanism:** When the WDT overflows, the chip understands that the software has completely frozen. To save itself, the ESP32 will force the system to restart immediately (Hard Reset), creating an infinite `0x10` loop.

---
## 📌 Khung Code Mẫu Chống Reset `0x10`
## 📌 Sample Code Frame to Prevent Reset `0x10`

Dưới đây là cấu trúc code thiết kế theo nguyên lý bất đồng bộ, sử dụng `millis()` thay thế hoàn toàn cho `delay()` truyền thống và chèn giải pháp giải phóng CPU core hợp lý để "nuôi" Watchdog.

```cpp
/*
 * ESP32 STABLE CORE CODE TEMPLATE
 * Thiết kế giải quyết triệt để lỗi rst:0x10
 */

unsigned long previousMillisTask = 0;
const long intervalTask = 1000; // Thực hiện tác vụ định kỳ mỗi 1000ms (1 giây)

void setup() {
  // BẮT BUỘC: Sử dụng Baudrate 115200 để đọc được phân tích log crash của ESP32
  Serial.begin(115200); 
  delay(1000); // Khoảng dừng ngắn cho Serial Monitor ổn định khi vừa cắm nguồn
  
  Serial.println("\n=== [START] ESP32 KHỞI ĐỘNG HỆ THỐNG ỔN ĐỊNH ===");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. CHẠY TÁC VỤ ĐỊNH KỲ KHÔNG GÂY NGHẼN (Non-blocking)
  if (currentMillis - previousMillisTask >= intervalTask) {
    previousMillisTask = currentMillis;
    Serial.println("[INFO] Hệ thống đang vận hành ổn định...");
  }

  // 2. XỬ LÝ VÒNG LẶP NẶNG AN TOÀN
  // Giả sử bạn có một vòng lặp xử lý dữ liệu thuật toán tốn thời gian
  for (int i = 0; i < 1000; i++) {
    // Thực hiện tính toán của bạn ở đây...
    
    // GIẢI PHÁP SỬA LỖI 0x10: 
    // Nhường core giải phóng cho Watchdog nếu vòng lặp chạy mất nhiều mili-giây
    yield(); 
  }

  // 3. ĐỆM GIẢI PHÓNG CPU
  // Một khoảng nghỉ siêu nhỏ (1ms) ở cuối loop giúp nuôi WDT của FreeRTOS cực kỳ hiệu quả
  delay(1); 
}
