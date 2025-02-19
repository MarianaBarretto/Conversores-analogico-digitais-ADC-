<h1>Controle de LEDs e Display OLED SSD1306 com Conversores Analógico-Digitais  - Embarcatech 💻</h1>
<p>Este projeto foi desenvolvido para controlar LEDs e um display OLED SSD1306 usando o microcontrolador RP2040. Ele utiliza entradas de botões e um joystick para controlar a interação com os LEDs e o display. A funcionalidade inclui a modulação por largura de pulso (PWM) para controlar o brilho dos LEDs e a exibição de bordas no display OLED.</p>

<h2>Componentes Necessários</h2>
<ul>
    <li><strong>RP2040</strong> (Placa Raspberry Pi Pico ou similar)</li>
    <li><strong>Display OLED SSD1306</strong> (128x64 pixels)</li>
    <li><strong>Joystick analógico</strong> (com botões e eixos X/Y)</li>
    <li><strong>LEDs RGB</strong> (Verde, Azul, Vermelho)</li>
    <li><strong>Botões</strong> (A e Joystick)</li>
    <li><strong>Resistores pull-up</strong> (para os botões)</li>
</ul>

<h2>Descrição das Funcionalidades</h2>

<h3>LEDs</h3>
<ul>
    <li><strong>LED Verde (Pino 11):</strong> Controlado diretamente por GPIO. Pode ser aceso ou apagado.</li>
    <li><strong>LED Azul (Pino 12):</strong> Controlado via PWM. O brilho é ajustado com base na posição Y do joystick.</li>
    <p>Quando o joystick estiver na posição central (valor 2048), o LED azul permanecerá apagado. À medida que o joystick for movido para cima (valores menores) ou para baixo (valores maiores), o brilho do LED aumentará gradualmente, atingindo a intensidade máxima nos extremos (0 e 4095).</p>    
    <li><strong>LED Vermelho (Pino 13):</strong> Controlado via PWM. O brilho é ajustado com base na posição X do joystick.</li>
    <p>Quando o joystick estiver na posição central (valor 2048), o LED vermelho permanecerá apagado. Ao mover o joystick para a esquerda (valores menores) ou para a direita (valores maiores), o brilho do LED aumentará gradualmente, sendo mais intenso nos extremos (0 e 4095).</p>
</ul>

<h3>Joystick</h3>
<p>O joystick é utilizado para controlar a posição de um quadrado no display OLED. A posição do quadrado é mapeada para os valores dos eixos X e Y do joystick. Além disso, o joystick possui um botão que alterna o estado do LED Verde entre aceso e apagado e altera a borda do Display:</p>
<ol>
    <li><strong>LED Verde aceso:</strong> A borda do display OLED é pontilhada</li>
    <li><strong>LED Verde apagado:</strong> A borda do display se torna tracejada.</li>
</ol>

<h3>Display OLED SSD1306</h3>
<ul>
    <li>Exibe um quadrado de 8x8 pixels que se move com base na posição do joystick.</li>
    <li>O estilo da borda do display pode ser alterado entre <strong>pontilhada</strong> e <strong>tracejada</strong>.</li>
    <li>O quadrado é desenhado dentro da borda com base nos valores de ADC dos eixos do joystick.</li>
</ul>

<h3>Botões</h3>
<ul>
    <li><strong>Botão A (Pino 5):</strong> Alterna entre ativar ou desativar os LED PWM a cada acionamento.</li>
    <li><strong>Botão do Joystick (Pino 22):</strong> Alterna o estado do LED Verde a cada acionamento, entre ligado e desligado e modifica a borda do display:</li>
    <ul>
        <li class="led-estado"><strong>LED Verde Ligado:</strong> Borda Pontilhada</li>
        <li class="led-estado"><strong>LED Verde Desligado:</strong> Borda Tracejada</li>
    </ul>
</ul>

<h3>Interrupções</h3>
<p>As interrupções são usadas para detectar quando os botões são pressionados. Isso evita múltiplas leituras rápidas e garante que as mudanças de estado aconteçam de forma controlada.</p>

<h3>PWM</h3>
<p>O PWM é utilizado para controlar o brilho dos LEDs azul e vermelho. O brilho é ajustado com base nas leituras dos eixos X e Y do joystick.</p>

<h2>Mapeamento de Pinos</h2>
<ul>
    <li><strong>LED Verde (Pino 11):</strong> Controlado por GPIO.</li>
    <li><strong>LED Azul (Pino 12):</strong> Controlado por PWM.</li>
    <li><strong>LED Vermelho (Pino 13):</strong> Controlado por PWM.</li>
    <li><strong>Joystick X (Pino 26):</strong> Eixo X do joystick (Entrada ADC).</li>
    <li><strong>Joystick Y (Pino 27):</strong> Eixo Y do joystick (Entrada ADC).</li>
    <li><strong>Botão A (Pino 5):</strong> Botão A (Entrada com pull-up).</li>
    <li><strong>Botão do Joystick (Pino 22):</strong> Botão do joystick (Entrada com pull-up).</li>
    <li><strong>SDA (Pino 14):</strong> Pino de dados para comunicação I2C com o display OLED.</li>
    <li><strong>SCL (Pino 15):</strong> Pino de relógio para comunicação I2C com o display OLED.</li>
</ul>

<h2>Bibliotecas Utilizadas</h2>
<ul>
    <li><strong>ssd1306.h:</strong> Biblioteca para controle do display OLED SSD1306.</li>
    <li><strong>adc.h:</strong> Biblioteca para controle do conversor analógico-digital (ADC).</li>
    <li><strong>pwm.h:</strong> Biblioteca para controle da modulação por largura de pulso (PWM).</li>
    <li><strong>i2c.h:</strong> Biblioteca para comunicação I2C.</li>
    <li><strong>gpio.h:</strong> Biblioteca para controle de pinos GPIO.</li>    
</ul>

<h2>Instruções de Compilação e Execução</h2>

<h3>1. Configuração do Ambiente:</h3>
<p>Certe-se de que o ambiente de desenvolvimento está configurado corretamente para o RP2040 (Raspberry Pi Pico) com o compilador <code>gcc</code> e a ferramenta <code>cmake</code>.</p>

<h3>2. Compilação:</h3>
<p>Clone o repositório contendo o código-fonte. Abra um terminal na pasta do projeto e execute os comandos abaixo:</p>
<pre><code>
mkdir build
cd build
cmake ..
make
</code></pre>

<h3>3. Carregamento no RP2040:</h3>
<p>Conecte a placa RP2040 ao computador no modo de gravação. Copie o arquivo <code>.uf2</code> gerado para a memória da placa.</p>

<h3>4. Execução:</h3>
<p>Após o código ser carregado na placa, o programa será iniciado automaticamente e controlará os LEDs e o display OLED conforme descrito.</p>

<h2>Imagens</h2>
<p align="center">
    <img src="LINK DA IMAGEM AQUI" alt="Projeto com RP2040 e Display OLED SSD1306" />
</p>

<h3>Vídeo de Demostração</h3>
<p>Google Drive: AQUI</p>
<p>YouTube: AQUI</p>

<h4>Autor: <strong>Mariana Barretto</strong></h4>
