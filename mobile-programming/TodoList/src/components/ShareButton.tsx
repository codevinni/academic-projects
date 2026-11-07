import { useState } from "react";
import { ShareButtonProps, Task } from "../types";


function ShareButton({ tasks }: ShareButtonProps) {

    const [copied, setCopied] = useState<boolean>(false);

    // Formata a lista inteira de tarefas
    function formatTasksToText(tasksList: Task[]): string {

        if (tasksList.length === 0)
            return "📋 Minha Lista de Tarefas:\n(Nenhuma tarefa cadastrada no momento)";

        const formattedLines = tasksList.map((task) => {
            const statusIcon = task.completed ? "✅" : "⬜";
            return `${statusIcon} ${task.description}`;
        });

        return `📋 Minha Lista de Tarefas:\n\n${formattedLines.join("\n")}`;
    }

    // Copia o texto para a Área de Transferência
    async function copyToClipboard(text: string) {

        if (!navigator.clipboard || !navigator.clipboard.writeText)
            return;

        try {

            await navigator.clipboard.writeText(text);
            setCopied(true);

            setTimeout(() => {
                setCopied(false);
            }, 2000);

        } catch (error) {
            console.error("Erro ao copiar lista para a área de transferência:", error);
        }
    }

    // Gerencia o fluxo de compartilhamento
    async function handleShare() {

        const textToShare = formatTasksToText(tasks);

        if (navigator.share) {
            try {

                await navigator.share({
                    title: "Minha Lista de Tarefas",
                    text: textToShare,
                });

                return;

            } catch (error) {
                if ((error as Error).name === "AbortError")
                    return;
            }
        }

        // Fallback para Área de Transferência
        await copyToClipboard(textToShare);
    }

    return (
        <button
            type="button"
            className={`share-button ${copied ? "share-button--copied" : ""}`}
            onClick={handleShare}
            aria-label={copied ? "Lista copiada para a área de transferência" : "Compartilhar lista de tarefas"}
            title={copied ? "Lista copiada!" : "Compartilhar lista"}
        >
            {copied ? (
                <>
                    {/* Ícone de Sucesso */}
                    <svg
                        className="share-button-icon share-button-icon--success"
                        width="18"
                        height="18"
                        viewBox="0 0 24 24"
                        fill="none"
                        stroke="currentColor"
                        strokeWidth="2.5"
                        strokeLinecap="round"
                        strokeLinejoin="round"
                    >
                        <polyline points="20 6 9 17 4 12"></polyline>
                    </svg>
                    <span className="share-button-text">Copiado!</span>
                </>
            ) : (
                /* Ícone de Compartilhamento Nativo */
                <svg
                    className="share-button-icon"
                    width="18"
                    height="18"
                    viewBox="0 0 24 24"
                    fill="none"
                    stroke="currentColor"
                    strokeWidth="2.2"
                    strokeLinecap="round"
                    strokeLinejoin="round"
                >
                    <circle cx="18" cy="5" r="3"></circle>
                    <circle cx="6" cy="12" r="3"></circle>
                    <circle cx="18" cy="19" r="3"></circle>
                    <line x1="8.59" y1="13.51" x2="15.42" y2="17.49"></line>
                    <line x1="15.41" y1="6.51" x2="8.59" y2="10.49"></line>
                </svg>
            )}
        </button>
    );
}

export default ShareButton;