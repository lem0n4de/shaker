from selenium.common.exceptions import WebDriverException
from seleniumwire import webdriver
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.common.by import By
from selenium.webdriver.support import expected_conditions as EC
import logging
from dataclasses import dataclass, field
from dataclass_wizard import JSONWizard
from typing import List
import uuid


@dataclass
class Video(JSONWizard):
    id: str
    name: str
    teacher: str = None
    url: str = None


@dataclass
class Lesson(JSONWizard):
    id: str
    teacher: str
    teacher_html_id: str
    html_id: str
    name: str
    videos: List[Video] = field(default_factory=list)


def scrape_lesson(driver: webdriver.Chrome, lesson: Lesson):
    try:
        video_element_list = []
        import time
        time.sleep(7)
        d_l = driver.find_element(By.CLASS_NAME, "DerslerListesi").find_elements(By.TAG_NAME, "a")
        if len(d_l) > 0:
            for d in d_l:
                video_element_list.append((d.text, d.get_attribute("id")))

        for video_element in video_element_list:
            driver.find_element(By.ID, video_element[1]).click()
            video_src = driver.find_element(By.ID, "Vid").get_attribute("src")
            lesson.videos.append(
                Video(id=str(uuid.uuid4()), name=video_element[0], url=video_src, teacher=lesson.teacher))
    except WebDriverException as e:
        logging.error(e.msg, lesson)


def scrape():
    opts = webdriver.ChromeOptions()
    opts.add_argument(
        "user-agent=Mozilla/5.0 (Macintosh; Intel Mac OS X 12_3) AppleWebKit/605.1.15 (KHTML, like Gecko) "
        "Version/15.3 Safari/605.1.15")
    driver = webdriver.Chrome(options=opts)
    assert (driver.execute_script("return navigator.userAgent"),
            "Mozilla/5.0 (Macintosh; Intel Mac OS X 12_3) AppleWebKit/605.1.15 (KHTML, like Gecko) "
            "Version/15.3 Safari/605.1.15")
    driver.maximize_window()

    try:
        driver.get("https://www.tusworld.com.tr/UyeGirisi")
        wait = WebDriverWait(driver, 300)
        wait.until(EC.url_matches("https://www.tusworld.com.tr/Anasayfa"))
        wait.until(EC.element_to_be_clickable((By.ID, "tclose")))
        driver.find_element(By.ID, "tclose").click()

        # driver.get("https://tusworld.com.tr/TusAnaKategori")

        wait.until(EC.element_to_be_clickable((By.CLASS_NAME, "VdRnk")))
        driver.find_element(By.CLASS_NAME, "VdRnk").click()

        wait.until(EC.element_to_be_clickable((By.CLASS_NAME, "Tusblue")))
        driver.find_element(By.CLASS_NAME, "Tusblue").click()

        wait.until(EC.presence_of_element_located((By.CLASS_NAME, "VdDrKaSub")))
        found = False
        for item in driver.find_elements(By.CLASS_NAME, "VdDrKaSub"):
            if "evde" in item.text.lower() and "offline" in item.text.lower():
                found = True
                item.click()
                break
        if not found:
            logging.error("VdDrKaSub bulunamadı.")
            return

        wait.until(EC.url_matches("https://www.tusworld.com.tr/VideoKategori"))
        found = False
        for item in driver.find_elements(By.TAG_NAME, "h1"):
            if "okul öğrencilerine premium paket" in item.text.lower():
                found = True
                item.click()
                break
        if not found:
            logging.error('"okul öğrencilerine premium paket" bulunamadı.')

        wait.until(EC.url_matches("https://www.tusworld.com.tr/VideoGrupDersleri"))
        __lessons = []
        for item in driver.find_element(
                By.CLASS_NAME, "DersKategorileri"
        ).find_elements(By.TAG_NAME, "a"):
            __lessons.append((item.get_attribute("id"), item.text))
        _lessons = []
        for lesson in __lessons:
            driver.find_element(By.ID, lesson[0]).find_element(By.TAG_NAME, "span").click()
            driver.timeouts.implicit_wait = 20
            try:
                hc_atf = driver.find_element(By.CLASS_NAME, "HcAtf")
                x = Lesson(id=str(uuid.uuid4()), html_id=lesson[0], name=lesson[1], teacher=hc_atf.text,
                           teacher_html_id=hc_atf.get_attribute("id"))
                _lessons.append(x)
                hc_alts = driver.find_elements(By.CLASS_NAME, "HocaAlt")
                y = [Lesson(id=str(uuid.uuid4()), html_id=lesson[0], name=lesson[1], teacher=hc.text,
                            teacher_html_id=hc.get_attribute("id")) for hc in hc_alts]
                _lessons.extend(y)
                scrape_lesson(driver, x)
                for l in y:
                    driver.find_element(By.ID, l.teacher_html_id).click()
                    scrape_lesson(driver, l)
            except WebDriverException as e:
                logging.warning(e.msg, f"HcAtf veya HocaAlt bulunamadı. {lesson}")
                l = Lesson(id=str(uuid.uuid4()), html_id=lesson[0], name=lesson[1], teacher=None, teacher_html_id=None)
                _lessons.append(l)
                scrape_lesson(driver, l)

        with open("lessons.json", "w+") as f:
            f.write(Lesson.list_to_json(_lessons))
    except WebDriverException as e:
        logging.exception(e.msg)
    finally:
        driver.close()


if __name__ == "__main__":
    scrape()
