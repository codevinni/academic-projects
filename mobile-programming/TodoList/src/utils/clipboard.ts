/**
 * Utilitário universal para cópia de texto para a Área de Transferência.
 *
 * Funciona de forma resiliente tanto em produção (HTTPS via Clipboard API moderna)
 * quanto em desenvolvimento / testes locais (HTTP na rede local do celular via IP ou Desktop)
 * utilizando o fallback com document.execCommand('copy').
 */
export async function copyToClipboard(text: string): Promise<boolean> {
    // 1. Tenta a Clipboard API moderna quando disponível
    if (
        typeof navigator !== "undefined" &&
        navigator.clipboard &&
        typeof navigator.clipboard.writeText === "function"
    ) {
        try {
            await navigator.clipboard.writeText(text);
            return true;
        } catch (err) {
            console.warn("navigator.clipboard.writeText falhou, acionando fallback:", err);
        }
    }

    // 2. Fallback universal usando textarea temporário + document.execCommand('copy')
    try {
        const textArea = document.createElement("textarea");
        textArea.value = text;

        // Estilos para não causar scroll nem zoom na tela
        textArea.style.position = "fixed";
        textArea.style.top = "0";
        textArea.style.left = "0";
        textArea.style.width = "2em";
        textArea.style.height = "2em";
        textArea.style.padding = "0";
        textArea.style.border = "none";
        textArea.style.outline = "none";
        textArea.style.boxShadow = "none";
        textArea.style.background = "transparent";
        textArea.style.opacity = "0";
        textArea.setAttribute("readonly", ""); // evita acionar o teclado virtual em dispositivos móveis

        document.body.appendChild(textArea);
        textArea.focus();
        textArea.select();
        textArea.setSelectionRange(0, text.length); // crucial para iOS Safari

        const successful = document.execCommand("copy");
        document.body.removeChild(textArea);
        return successful;
    } catch (err) {
        console.error("Falha no fallback de cópia via execCommand:", err);
        return false;
    }
}

