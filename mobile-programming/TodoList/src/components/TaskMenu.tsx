import { useState, useRef, useEffect } from "react";
import { TaskMenuProps } from "../types";

function TaskMenu({ task }: TaskMenuProps) {

    const [isOpen, setIsOpen] = useState<boolean>(false);
    const [copied, setCopied] = useState<boolean>(false);
    const menuRef = useRef<HTMLDivElement>(null);

    // Fecha o menu caso o usuário toque/clique fora dele
    useEffect(() => {

        function handleClickOutside(event: MouseEvent | TouchEvent) {
            if (menuRef.current && !menuRef.current.contains(event.target as Node))
                setIsOpen(false);
        }

        if (isOpen)
            document.addEventListener("pointerdown", handleClickOutside);

        return () => {
            document.removeEventListener("pointerdown", handleClickOutside);
        };

    }, [isOpen]);

    // Alterna a abertura do menu de opções
    function handleToggleMenu(e: React.MouseEvent) {
        e.stopPropagation();
        setIsOpen((prev) => !prev);
    }

    // Copia o texto da tarefa para a Área de Transferência
    async function handleCopyText() {

        if (navigator.clipboard && navigator.clipboard.writeText) {

            try {

                await navigator.clipboard.writeText(task.description);
                setCopied(true);

                // Exibe o feedback "Copiado!" por 2 segundos e fecha o menu
                setTimeout(() => {
                    setCopied(false);
                    setIsOpen(false);
                }, 2000);

            } catch (error) {
                setIsOpen(false);
            }

        } else {
            setIsOpen(false);
        }
    }

    const menuActions = [

        {
            id: "copy",
            label: copied ? "Copiado!" : "Copiar texto",
            isSuccess: copied,
            icon: copied ? (

                // Ícone de Sucesso
                <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                    strokeWidth="2.4" strokeLinecap="round" strokeLinejoin="round">
                    <polyline points="20 6 9 17 4 12"></polyline>
                </svg>

            ) : (

                // Ícone de Copiar
                <svg width="15" height="15" viewBox="0 0 24 24" fill="none" stroke="currentColor"
                    strokeWidth="2.2" strokeLinecap="round" strokeLinejoin="round">
                    <rect x="9" y="9" width="13" height="13" rx="2" ry="2"></rect>
                    <path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"></path>
                </svg>
            ),
            onClick: handleCopyText,
        },

        /* Novas opções podem ser adicionadas aqui:
        {
            id: "edit",
            label: "Editar tarefa",
            isSuccess: false,
            icon: <svg>...</svg>,
            onClick: handleEditTask,
        }
        */
    ];

    return (
        <div
            className="task-menu-container"
            ref={menuRef}
            onClick={(e) => e.stopPropagation()}
            onKeyDown={(e) => e.stopPropagation()}
        >
            {/* Botão Gatilho (3 Pontinhos Verticais) */}
            <button
                type="button"
                className={`task-menu-btn ${task.completed ? "task-menu-btn--completed" : "task-menu-btn--pending"
                    } ${isOpen ? "task-menu-btn--open" : ""}`}
                onClick={handleToggleMenu}
                aria-label="Opções da tarefa"
                aria-haspopup="true"
                aria-expanded={isOpen}
                title="Opções"
            >
                <svg
                    width="18"
                    height="18"
                    viewBox="0 0 24 24"
                    fill="none"
                    stroke="currentColor"
                    strokeWidth="2.4"
                    strokeLinecap="round"
                    strokeLinejoin="round"
                >
                    <circle cx="12" cy="12" r="1.5" fill="currentColor"></circle>
                    <circle cx="12" cy="5" r="1.5" fill="currentColor"></circle>
                    <circle cx="12" cy="19" r="1.5" fill="currentColor"></circle>
                </svg>
            </button>

            {/* Menu Popover Suspenso */}
            {isOpen && (
                <div className="task-menu-dropdown" role="menu">
                    {menuActions.map((action) => (
                        <button
                            key={action.id}
                            type="button"
                            className={`task-menu-item ${action.isSuccess ? "task-menu-item--success" : ""
                                }`}
                            onClick={(e) => {
                                e.stopPropagation();
                                action.onClick();
                            }}
                            role="menuitem"
                        >
                            <span
                                className={`task-menu-item-icon ${action.isSuccess ? "task-menu-item-icon--success" : ""
                                    }`}
                            >
                                {action.icon}
                            </span>
                            <span
                                className={`task-menu-item-text ${action.isSuccess ? "task-menu-item-text--success" : ""
                                    }`}
                            >
                                {action.label}
                            </span>
                        </button>
                    ))}
                </div>
            )}
        </div>
    );
}

export default TaskMenu;
