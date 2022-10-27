from selenium import webdriver 
import random
from time import sleep
from selenium.webdriver.common.by import By
import logging
from selenium.webdriver.remote.remote_connection import LOGGER
LOGGER.setLevel(logging.WARNING)
logging.getLogger("urllib3").setLevel(logging.WARNING)
logging.getLogger("DevTools").setLevel(logging.WARNING)

url = 'https://www.netflix.com/browse'

email = 'thiago00silva00melo@gmail.com'
senha = 'Tima123456'

generos = ["romance", "animes", "comedia", "ação", "drama", "policiais"]

#-----------------------------------------------------------#
#----------------Script to choose a random movie------------#
#-----------------------------------------------------------#
def movie(name):
	chrome_options = webdriver.ChromeOptions()
	pref = {"profile.default_content_setting_values.notifications":2}
	chrome_options.add_experimental_option("prefs",pref)
	chrome_options.add_argument('--disable-logging') 
	
	driver = webdriver.Chrome(chrome_options=chrome_options,
							 executable_path="C:/webdrivers/chromedriver")
	driver.get(url)
	driver.maximize_window()

	login = driver.find_element(By.ID, "id_userLoginId")
	login.send_keys(email)
	password = driver.find_element(By.ID, "id_password")
	password.send_keys(senha)
	
	driver.find_element(By.XPATH, '//*[@id="appMountPoint"]/div/div[3]/div/div/div[1]/form/button').click()

	sleep(5)
    
	driver.find_element(By.XPATH,'//*[@id="appMountPoint"]/div/div/div[1]/div[1]/div[2]/div/div/ul/li[1]/div/a').click()

	source = driver.find_element(By.XPATH,'//*[@id="appMountPoint"]/div/div/div[1]/div[1]/div[1]/div/div/div/div[1]/div/button')
	source.click()

	genero_escolhido = random.choice(generos)

	input_genero = driver.find_element(By.XPATH,'//*[@id="appMountPoint"]/div/div/div[1]/div[1]/div[1]/div/div/div/div[1]/div/div/input')
	input_genero.send_keys(genero_escolhido)

	
	sleep(6)

	linhas = list(range(0, 50))
	linha_escolhida = random.choice(linhas)
    
	filme = driver.find_element(By.XPATH,'//*[@id="title-card-0-{0}"]/div[1]/a'.format(linha_escolhida))

	for letra in str(filme.text):
		name.append(letra)
	filme.click()
	sleep(2)
	
	assistir = driver.find_element(By.CLASS_NAME,'previewModal--player-titleTreatmentWrapper')

	assistir.click()

	sleep(10000)

